#include "f1x/openauto/autoapp/UI/UpdateManager.hpp"
#include <qloggingcategory.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>

Q_LOGGING_CATEGORY(lcUpdate, "journeyos.ota")

namespace f1x::openauto::autoapp::UI {

UpdateManager::UpdateManager(configuration::IConfiguration::Pointer config, QObject *parent)
    : QObject(parent)
    , m_config(std::move(config))
{
    // Automatically check for updates on startup.
    // We use singleShot(0) to ensure signals are connected before the check runs.
    QTimer::singleShot(0, this, &UpdateManager::checkForUpdate);
}

QString UpdateManager::getCurrentVersion() const
{
    // Single source of truth: populated from the CMakeLists.txt version string via a define.
    // Falls back to a human-readable placeholder when the define is absent (dev builds).
#ifdef OPENAUTO_VERSION_STRING
    return QStringLiteral(OPENAUTO_VERSION_STRING);
#else
    return QStringLiteral("dev");
#endif
}

void UpdateManager::checkForUpdate()
{
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

    const QString serverUrl = QStringLiteral("https://updates.journeyos.org/os/ota/stable.json");
    qInfo(lcUpdate) << "checking for update url=" << serverUrl;

    QNetworkRequest request{QUrl(serverUrl)};
    request.setTransferTimeout(15000); // never hang longer than 15 s

    auto* networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this,
            [this, networkManager](QNetworkReply* reply) {
        // Schedule cleanup — deleteLater() defers to next event loop tick so
        // reply remains valid for the entire body of this callback.
        networkManager->deleteLater();
        reply->deleteLater();

        try {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning(lcUpdate) << "update check network error:" << reply->errorString();
                emit checkComplete(false, getCurrentVersion());
                return;
            }

            // Reject non-200 HTTP responses
            const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (httpStatus != 200) {
                qWarning(lcUpdate) << "update check unexpected HTTP status=" << httpStatus;
                emit checkComplete(false, getCurrentVersion());
                return;
            }

            const QByteArray data = reply->readAll();
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isNull() || !doc.isObject()) {
                qWarning(lcUpdate) << "update check: malformed JSON response";
                emit checkComplete(false, getCurrentVersion());
                return;
            }

            const QJsonObject obj = doc.object();
            const QString remoteVersion = obj.value(QStringLiteral("version")).toString();
            const QString downloadUrl   = obj.value(QStringLiteral("url")).toString();

            if (remoteVersion.isEmpty() || downloadUrl.isEmpty()) {
                qWarning(lcUpdate) << "update check: manifest missing version or url";
                emit checkComplete(false, getCurrentVersion());
                return;
            }

            m_latestVersion    = remoteVersion;
            m_updateUrl        = downloadUrl;
            m_updateAvailable  = (m_latestVersion != getCurrentVersion());

            emit updateAvailableChanged();
            emit latestVersionChanged();
            emit checkComplete(m_updateAvailable, m_latestVersion);

        } catch (const std::exception &e) {
            qCritical(lcUpdate) << "update check callback threw exception:" << e.what();
            emit checkComplete(false, getCurrentVersion());
        } catch (...) {
            qCritical(lcUpdate) << "update check callback threw unknown exception";
            emit checkComplete(false, getCurrentVersion());
        }
    });

    networkManager->get(request);
}

void UpdateManager::downloadUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "download called with no update URL";
        return;
    }
    qInfo(lcUpdate) << "download url=" << m_updateUrl;

    QNetworkRequest request{QUrl(m_updateUrl)};
    request.setTransferTimeout(300000); // 5 min — bundles can be large

    auto* networkManager = new QNetworkAccessManager(this);

    connect(networkManager, &QNetworkAccessManager::finished, this,
            [this, networkManager](QNetworkReply* reply) {
        networkManager->deleteLater();
        reply->deleteLater();

        try {
            if (reply->error() != QNetworkReply::NoError) {
                qWarning(lcUpdate) << "download failed:" << reply->errorString();
                emit errorOccurred(reply->errorString());
                return;
            }

            const QString destPath = QStandardPaths::writableLocation(
                                         QStandardPaths::TempLocation) + QStringLiteral("/update.raucb");
            QFile file(destPath);
            if (!file.open(QIODevice::WriteOnly)) {
                const QString msg = QStringLiteral("Failed to open %1 for writing").arg(destPath);
                qWarning(lcUpdate) << msg;
                emit errorOccurred(msg);
                return;
            }
            file.write(reply->readAll());
            file.close();
            qInfo(lcUpdate) << "download complete path=" << destPath;
            emit downloadFinished();

        } catch (const std::exception &e) {
            qCritical(lcUpdate) << "download callback threw exception:" << e.what();
            emit errorOccurred(QString::fromUtf8(e.what()));
        } catch (...) {
            qCritical(lcUpdate) << "download callback threw unknown exception";
            emit errorOccurred(QStringLiteral("Unknown error during download"));
        }
    });

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &UpdateManager::downloadProgress);
}

void UpdateManager::applyUpdate()
{
    QString bundlePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.raucb";
    if (!QFile::exists(bundlePath)) {
        qWarning(lcUpdate) << "apply called but bundle not found at" << bundlePath;
        emit errorOccurred("Update bundle not found.");
        return;
    }
    qInfo(lcUpdate) << "Applying update from" << bundlePath;

    // Execute RAUC install. Note: This usually requires root permissions or appropriate capabilities.
    QProcess* process = new QProcess(this);

    // Handle successful execution (or clean exit with error code)
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            qInfo(lcUpdate) << "RAUC install success";
            emit installFinished(true, "Installation successful. System will reboot.");
        } else {
            qWarning(lcUpdate) << "RAUC install failed with code" << exitCode;
            emit installFinished(false, QString("Installer failed with code %1").arg(exitCode));
        }
    });

    // Handle failure to start (e.g. rauc binary not found)
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        process->deleteLater();
        qWarning(lcUpdate) << "RAUC process failed to start:" << error;
        emit installFinished(false, "Failed to start installer process.");
    });

    process->start("rauc", QStringList() << "install" << bundlePath);
}

} // namespace
