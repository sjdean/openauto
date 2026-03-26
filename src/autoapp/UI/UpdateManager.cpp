#include "f1x/openauto/autoapp/UI/UpdateManager.hpp"
#include <qloggingcategory.h>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>

Q_LOGGING_CATEGORY(lcUpdate, "journeyos.ota")

namespace f1x::openauto::autoapp::UI {

// ---------------------------------------------------------------------------
// UpdateCheckWorker
//
// Lives on its own QThread. Owns its QNetworkAccessManager (created on the
// worker thread so it is affiliated to the correct thread).
// Retries up to MAX_ATTEMPTS times with exponential back-off. Each attempt
// checks network reachability first so we don't waste socket resources
// on an interface that is clearly offline.
// ---------------------------------------------------------------------------
class UpdateCheckWorker : public QObject {
    Q_OBJECT
public:
    static constexpr int    MAX_ATTEMPTS      = 3;
    static constexpr int    TIMEOUT_MS        = 15000; // per-attempt connect/transfer timeout
    static constexpr int    RETRY_DELAY_MS[3] = {0, 30000, 120000}; // backoff: immediate, 30 s, 2 min

    explicit UpdateCheckWorker(QObject *parent = nullptr) : QObject(parent) {}

signals:
    // Success path — emit parsed manifest fields.
    void checkResult(bool available, const QString &latestVersion, const QString &downloadUrl);
    // Failure path — emit human-readable reason (already logged).
    void checkFailed(const QString &reason);

public slots:
    void run()
    {
        m_attempt = 0;
        m_nam     = new QNetworkAccessManager(this);
        doAttempt();
    }

private:
    void doAttempt()
    {
        // ---- 1. Reachability guard ----
        if (!hasUsableNetwork()) {
            if (m_attempt + 1 < MAX_ATTEMPTS) {
                scheduleRetry(QStringLiteral("no usable network interface"));
            } else {
                fail(QStringLiteral("no usable network interface after %1 attempt(s)")
                         .arg(MAX_ATTEMPTS));
            }
            return;
        }

        const QString url = QStringLiteral("https://updates.journeyos.org/os/ota/stable.json");
        qInfo(lcUpdate) << "check attempt" << (m_attempt + 1) << "/" << MAX_ATTEMPTS
                        << "url=" << url;

        QNetworkRequest req{QUrl(url)};
        req.setTransferTimeout(TIMEOUT_MS);

        QNetworkReply *reply = m_nam->get(req);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            try {
                handleReply(reply);
            } catch (const std::exception &e) {
                qCritical(lcUpdate) << "check reply handler threw:" << e.what();
                retryOrFail(QString::fromUtf8(e.what()));
            } catch (...) {
                qCritical(lcUpdate) << "check reply handler threw unknown exception";
                retryOrFail(QStringLiteral("unknown exception in reply handler"));
            }
        });
    }

    void handleReply(QNetworkReply *reply)
    {
        // ---- 2. Transport-level error ----
        if (reply->error() != QNetworkReply::NoError) {
            const QString msg = reply->errorString();
            qWarning(lcUpdate) << "network error attempt" << (m_attempt + 1) << ":" << msg;
            retryOrFail(msg);
            return;
        }

        // ---- 3. HTTP status ----
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatus != 200) {
            const QString msg = QStringLiteral("unexpected HTTP status %1").arg(httpStatus);
            qWarning(lcUpdate) << msg << "attempt" << (m_attempt + 1);
            retryOrFail(msg);
            return;
        }

        // ---- 4. JSON parse ----
        const QByteArray data = reply->readAll();
        QJsonParseError parseErr;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
        if (doc.isNull() || !doc.isObject()) {
            const QString msg = QStringLiteral("malformed JSON: %1").arg(parseErr.errorString());
            qWarning(lcUpdate) << msg << "attempt" << (m_attempt + 1);
            retryOrFail(msg);
            return;
        }

        // ---- 5. Manifest validation ----
        const QJsonObject obj = doc.object();
        const QString remoteVersion = obj.value(QStringLiteral("version")).toString();
        const QString downloadUrl   = obj.value(QStringLiteral("url")).toString();

        if (remoteVersion.isEmpty() || downloadUrl.isEmpty()) {
            const QString msg = QStringLiteral("manifest missing version or url");
            qWarning(lcUpdate) << msg;
            retryOrFail(msg);
            return;
        }

        qInfo(lcUpdate) << "check succeeded remote=" << remoteVersion << "url=" << downloadUrl;
        emit checkResult(/*available=*/true, remoteVersion, downloadUrl);
    }

    void retryOrFail(const QString &reason)
    {
        ++m_attempt;
        if (m_attempt < MAX_ATTEMPTS) {
            scheduleRetry(reason);
        } else {
            fail(reason);
        }
    }

    void scheduleRetry(const QString &reason)
    {
        const int delayMs = (m_attempt < 3) ? RETRY_DELAY_MS[m_attempt] : RETRY_DELAY_MS[2];
        qInfo(lcUpdate) << "will retry in" << delayMs << "ms, reason:" << reason;
        QTimer::singleShot(delayMs, this, &UpdateCheckWorker::doAttempt);
    }

    void fail(const QString &reason)
    {
        qWarning(lcUpdate) << "check failed after" << MAX_ATTEMPTS << "attempt(s):" << reason;
        emit checkFailed(reason);
    }

    // Returns true if at least one non-loopback, non-VM interface is up
    // with a valid unicast address — a lightweight proxy for "we might have internet".
    static bool hasUsableNetwork()
    {
        const auto ifaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &iface : ifaces) {
            if (!(iface.flags() & QNetworkInterface::IsUp))
                continue;
            if (iface.flags() & QNetworkInterface::IsLoopBack)
                continue;
            const auto entries = iface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (!entry.ip().isNull() && !entry.ip().isLoopback())
                    return true;
            }
        }
        return false;
    }

    QNetworkAccessManager *m_nam{nullptr};
    int                    m_attempt{0};
};

// ---------------------------------------------------------------------------
// UpdateManager
// ---------------------------------------------------------------------------

UpdateManager::UpdateManager(configuration::IConfiguration::Pointer config, QObject *parent)
    : QObject(parent)
    , m_config(std::move(config))
{
    // Create the worker and move it to a dedicated thread.
    m_checkThread = new QThread(this);
    m_checkThread->setObjectName(QStringLiteral("ota-check"));

    m_checkWorker = new UpdateCheckWorker();
    m_checkWorker->moveToThread(m_checkThread);

    // Worker → manager connections (queued across thread boundary automatically).
    connect(m_checkWorker, &UpdateCheckWorker::checkResult,
            this, &UpdateManager::onCheckResult);
    connect(m_checkWorker, &UpdateCheckWorker::checkFailed,
            this, &UpdateManager::onCheckFailed);

    // Manager → worker trigger (queued: safe to call from main thread).
    connect(this, &UpdateManager::startCheckRequested,
            m_checkWorker, &UpdateCheckWorker::run, Qt::QueuedConnection);

    // Clean up the worker object when the thread finishes.
    connect(m_checkThread, &QThread::finished,
            m_checkWorker, &QObject::deleteLater);

    m_checkThread->start();

    // Kick off the first check after the event loop starts so all signal
    // connections in main.cpp are established before we emit anything.
    QTimer::singleShot(0, this, &UpdateManager::checkForUpdate);
}

UpdateManager::~UpdateManager()
{
    if (m_checkThread) {
        m_checkThread->quit();
        if (!m_checkThread->wait(20000)) {
            qWarning(lcUpdate) << "OTA check thread did not finish in time — terminating";
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
        const bool desktopMode = m_config->getSettingByName<bool>("System", "DesktopMode", false);
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

    const QString destPath = QStandardPaths::writableLocation(
                                 QStandardPaths::TempLocation) + QStringLiteral("/update.raucb");

    auto *file = new QFile(destPath, this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString msg = QStringLiteral("Failed to open %1 for writing").arg(destPath);
        qWarning(lcUpdate) << msg;
        file->deleteLater();
        emit errorOccurred(msg);
        return;
    }

    QNetworkRequest request{QUrl(m_updateUrl)};
    request.setTransferTimeout(300000); // 5 min global timeout

    auto *networkManager = new QNetworkAccessManager(this);
    QNetworkReply *reply = networkManager->get(request);

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
                QFile::remove(destPath); // clean up partial file
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
    const QString bundlePath = QStandardPaths::writableLocation(
                                   QStandardPaths::TempLocation) + QStringLiteral("/update.raucb");
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
            emit installFinished(true, QStringLiteral("Installation successful. System will reboot."));
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

// Required because UpdateCheckWorker is defined in this .cpp file
// and uses Q_OBJECT.
#include "UpdateManager.moc"
