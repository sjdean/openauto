#include "f1x/openauto/autoapp/UI/UpdateManager.hpp"
#include <qloggingcategory.h>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>

Q_LOGGING_CATEGORY(lcUpdate, "journeyos.ota")

namespace f1x::openauto::autoapp::UI {
using configuration::ConfigGroup;
using configuration::ConfigKey;

// ---------------------------------------------------------------------------
// UpdateCheckWorker
//
// Runs a synchronous-style check loop on its own QThread (via moveToThread).
// Uses a per-request QEventLoop so the network stack has an event loop to
// drive it, while keeping the control flow linear and easy to reason about.
//
// Retry schedule: immediate → 30 s → 2 min
// Every potential failure mode is caught — no exception may escape run().
// ---------------------------------------------------------------------------
class UpdateCheckWorker : public QObject {
    Q_OBJECT

    static constexpr int MAX_ATTEMPTS  = 3;
    static constexpr int TIMEOUT_MS    = 15000;
    static constexpr int RETRY_DELAY[] = {0, 30000, 120000};

public:
    explicit UpdateCheckWorker(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void checkResult(bool available, const QString &latestVersion, const QString &downloadUrl);
    void checkFailed(const QString &reason);

public slots:
    void run()
    {
        try {
            runImpl();
        } catch (const std::exception &e) {
            qCritical(lcUpdate) << "update check threw:" << e.what();
            emit checkFailed(QString::fromUtf8(e.what()));
        } catch (...) {
            qCritical(lcUpdate) << "update check threw unknown exception";
            emit checkFailed(QStringLiteral("unknown exception"));
        }
    }

private:
    void runImpl()
    {
        for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {

            // ---- Inter-attempt delay (skipped on first try) ----
            if (attempt > 0) {
                const int delayMs = RETRY_DELAY[attempt];
                qInfo(lcUpdate) << "retry" << attempt << "/" << (MAX_ATTEMPTS - 1)
                                << "— waiting" << delayMs << "ms";
                interruptibleSleep(delayMs);
                if (QThread::currentThread()->isInterruptionRequested()) {
                    qInfo(lcUpdate) << "check interrupted during retry delay";
                    emit checkFailed(QStringLiteral("interrupted"));
                    return;
                }
            }

            // ---- Network reachability guard ----
            if (!hasUsableNetwork()) {
                qInfo(lcUpdate) << "no usable network — attempt" << (attempt + 1)
                                << "/" << MAX_ATTEMPTS;
                continue; // count as an attempt, try again after back-off
            }

            qInfo(lcUpdate) << "check attempt" << (attempt + 1) << "/" << MAX_ATTEMPTS;

            // ---- HTTP GET with local event loop ----
            // QNetworkAccessManager is created here so it is affiliated with
            // the worker thread's event loop (the QEventLoop we exec below).
            QNetworkAccessManager nam;
            QNetworkRequest req{QUrl(
                QStringLiteral("https://updates.journeyos.org/os/ota/stable.json"))};
            req.setTransferTimeout(TIMEOUT_MS);

            QNetworkReply *reply = nam.get(req);

            QEventLoop loop;
            connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec(); // blocks this thread; network I/O runs inside here

            if (QThread::currentThread()->isInterruptionRequested()) {
                reply->abort();
                reply->deleteLater();
                qInfo(lcUpdate) << "check interrupted during request";
                emit checkFailed(QStringLiteral("interrupted"));
                return;
            }

            // ---- Transport error ----
            if (reply->error() != QNetworkReply::NoError) {
                qWarning(lcUpdate) << "network error attempt" << (attempt + 1)
                                   << ":" << reply->errorString();
                reply->deleteLater();
                continue;
            }

            // ---- HTTP status ----
            const int httpStatus =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (httpStatus != 200) {
                qWarning(lcUpdate) << "HTTP" << httpStatus << "— attempt" << (attempt + 1);
                reply->deleteLater();
                continue;
            }

            // ---- JSON parse ----
            const QByteArray data = reply->readAll();
            reply->deleteLater();

            QJsonParseError parseErr;
            const QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
            if (doc.isNull() || !doc.isObject()) {
                qWarning(lcUpdate) << "malformed JSON attempt" << (attempt + 1)
                                   << ":" << parseErr.errorString();
                continue;
            }

            // ---- Manifest validation ----
            const QJsonObject obj         = doc.object();
            const QString remoteVersion   = obj.value(QStringLiteral("version")).toString();
            const QString downloadUrl     = obj.value(QStringLiteral("url")).toString();

            if (remoteVersion.isEmpty() || downloadUrl.isEmpty()) {
                qWarning(lcUpdate) << "manifest missing version or url — attempt" << (attempt + 1);
                continue;
            }

            // ---- Success ----
            qInfo(lcUpdate) << "check succeeded remote=" << remoteVersion
                            << "url=" << downloadUrl;
            emit checkResult(true, remoteVersion, downloadUrl);
            return;
        }

        // All attempts exhausted without success.
        qWarning(lcUpdate) << "check failed after" << MAX_ATTEMPTS << "attempt(s)";
        emit checkFailed(QStringLiteral("all attempts exhausted"));
    }

    // Sleeps for `ms` milliseconds but wakes early if the thread is interrupted.
    // Polls every 200 ms so shutdown is responsive without tight-spinning.
    static void interruptibleSleep(int ms)
    {
        constexpr int TICK_MS = 200;
        for (int elapsed = 0; elapsed < ms; elapsed += TICK_MS) {
            if (QThread::currentThread()->isInterruptionRequested()) return;
            QThread::msleep(static_cast<unsigned long>(
                std::min(TICK_MS, ms - elapsed)));
        }
    }

    // Returns true if at least one non-loopback interface is up with a
    // non-null unicast address — a lightweight proxy for "internet possible".
    static bool hasUsableNetwork()
    {
        const auto ifaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &iface : ifaces) {
            if (!(iface.flags() & QNetworkInterface::IsUp))       continue;
            if (iface.flags()   & QNetworkInterface::IsLoopBack)  continue;
            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                if (!entry.ip().isNull() && !entry.ip().isLoopback())
                    return true;
            }
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
// UpdateManager
// ---------------------------------------------------------------------------

UpdateManager::UpdateManager(configuration::IConfiguration::Pointer config, QObject *parent)
    : QObject(parent)
    , m_config(std::move(config))
{
    m_checkThread = new QThread(this);
    m_checkThread->setObjectName(QStringLiteral("ota-check"));

    m_checkWorker = new UpdateCheckWorker();
    m_checkWorker->moveToThread(m_checkThread);

    connect(m_checkWorker, &UpdateCheckWorker::checkResult,
            this, &UpdateManager::onCheckResult);
    connect(m_checkWorker, &UpdateCheckWorker::checkFailed,
            this, &UpdateManager::onCheckFailed);

    connect(this, &UpdateManager::startCheckRequested,
            m_checkWorker, &UpdateCheckWorker::run, Qt::QueuedConnection);

    connect(m_checkThread, &QThread::finished,
            m_checkWorker, &QObject::deleteLater);

    m_checkThread->start();

    // Deferred so all QML signal connections are established first.
    QTimer::singleShot(0, this, &UpdateManager::checkForUpdate);
}

UpdateManager::~UpdateManager()
{
    if (m_checkThread) {
        m_checkThread->requestInterruption(); // wake interruptibleSleep
        m_checkThread->quit();
        if (!m_checkThread->wait(5000)) {
            qWarning(lcUpdate) << "OTA thread did not stop in time — terminating";
            m_checkThread->terminate();
            m_checkThread->wait(3000);
        }
    }
}

QString UpdateManager::getCurrentVersion() const
{
#ifdef OPENAUTO_VERSION_STRING
    return QStringLiteral(OPENAUTO_VERSION_STRING);
#else
    return QStringLiteral("dev");
#endif
}

void UpdateManager::checkForUpdate()
{
    if (m_checking) {
        qInfo(lcUpdate) << "check already in progress — skipping";
        return;
    }

    try {
        const bool desktopMode =
            m_config->getSettingByName<bool>(ConfigGroup::System, ConfigKey::SystemDesktopMode, false);
        if (desktopMode) {
            qInfo(lcUpdate) << "desktop mode: OTA disabled";
            emit checkComplete(false, getCurrentVersion());
            return;
        }
    } catch (const std::exception &e) {
        qWarning(lcUpdate) << "checkForUpdate: config read failed:" << e.what();
        emit checkComplete(false, getCurrentVersion());
        return;
    } catch (...) {
        qWarning(lcUpdate) << "checkForUpdate: config read threw unknown exception";
        emit checkComplete(false, getCurrentVersion());
        return;
    }

    m_checking = true;
    emit startCheckRequested();
}

void UpdateManager::onCheckResult(bool /*available*/, const QString &latestVersion,
                                   const QString &downloadUrl)
{
    m_checking        = false;
    m_latestVersion   = latestVersion;
    m_updateUrl       = downloadUrl;
    m_updateAvailable = (m_latestVersion != getCurrentVersion());

    emit updateAvailableChanged();
    emit latestVersionChanged();
    emit checkComplete(m_updateAvailable, m_latestVersion);
}

void UpdateManager::onCheckFailed(const QString &reason)
{
    m_checking = false;
    qWarning(lcUpdate) << "update check failed:" << reason;
    emit checkComplete(false, getCurrentVersion());
}

void UpdateManager::downloadUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "download called with no update URL";
        return;
    }
    qInfo(lcUpdate) << "download url=" << m_updateUrl;

    const QString destPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
        + QStringLiteral("/update.raucb");

    auto *file = new QFile(destPath, this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString msg =
            QStringLiteral("Failed to open %1 for writing").arg(destPath);
        qWarning(lcUpdate) << msg;
        file->deleteLater();
        emit errorOccurred(msg);
        return;
    }

    QNetworkRequest request{QUrl(m_updateUrl)};
    request.setTransferTimeout(300000); // 5 min global timeout

    auto *networkManager = new QNetworkAccessManager(this);
    QNetworkReply *reply  = networkManager->get(request);

    // Stream to disk — avoids buffering the entire bundle in RAM.
    connect(reply, &QNetworkReply::readyRead, this, [reply, file]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress,
            this, &UpdateManager::downloadProgress);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, file, networkManager, destPath]() {
        networkManager->deleteLater();
        reply->deleteLater();
        file->close();
        file->deleteLater();

        try {
            if (reply->error() != QNetworkReply::NoError) {
                const QString msg = reply->errorString();
                qWarning(lcUpdate) << "download failed:" << msg;
                QFile::remove(destPath);
                emit errorOccurred(msg);
                return;
            }
            qInfo(lcUpdate) << "download complete path=" << destPath;
            emit downloadFinished();
        } catch (const std::exception &e) {
            qCritical(lcUpdate) << "download callback threw:" << e.what();
            QFile::remove(destPath);
            emit errorOccurred(QString::fromUtf8(e.what()));
        } catch (...) {
            qCritical(lcUpdate) << "download callback threw unknown exception";
            QFile::remove(destPath);
            emit errorOccurred(QStringLiteral("Unknown error during download"));
        }
    });
}

void UpdateManager::applyUpdate()
{
    const QString bundlePath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
        + QStringLiteral("/update.raucb");

    if (!QFile::exists(bundlePath)) {
        qWarning(lcUpdate) << "apply called but bundle not found at" << bundlePath;
        emit errorOccurred(QStringLiteral("Update bundle not found."));
        return;
    }
    qInfo(lcUpdate) << "applying update from" << bundlePath;

    QProcess *process = new QProcess(this);

    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            qInfo(lcUpdate) << "RAUC install success";
            emit installFinished(true,
                QStringLiteral("Installation successful. System will reboot."));
        } else {
            qWarning(lcUpdate) << "RAUC install failed with code" << exitCode;
            emit installFinished(false,
                QStringLiteral("Installer failed with code %1").arg(exitCode));
        }
    });

    connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError error) {
        process->deleteLater();
        qWarning(lcUpdate) << "RAUC process error:" << error;
        emit installFinished(false, QStringLiteral("Failed to start installer process."));
    });

    process->start(QStringLiteral("rauc"),
                   QStringList() << QStringLiteral("install") << bundlePath);
}

} // namespace

#include "UpdateManager.moc"