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
    const bool desktopMode = m_config->getSettingByName<bool>("System", "DesktopMode", false);
    if (desktopMode) {
        qInfo(lcUpdate) << "Desktop mode detected. Updates (RAUC) are disabled.";
        emit checkComplete(false, getCurrentVersion());
        return;
    }

    const QString serverUrl = "https://updates.journeyos.org/os/ota/stable.json";
    //m_config->getSettingByName<QString>("Updates", "ServerUrl", "https://updates.journeyos.org/os/ota/stable.json");

    qInfo(lcUpdate) << "checking for update url=" << serverUrl;

    auto* networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, [this, networkManager](QNetworkReply* reply) {
        networkManager->deleteLater();
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning(lcUpdate) << "Update check failed:" << reply->errorString();
            emit checkComplete(false, getCurrentVersion());
            return;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject jsonObject = jsonResponse.object();

        QString remoteVersion = jsonObject["version"].toString();
        QString downloadUrl = jsonObject["url"].toString();

        if (remoteVersion.isEmpty() || downloadUrl.isEmpty()) {
            qWarning(lcUpdate) << "Invalid update manifest received";
            emit checkComplete(false, getCurrentVersion());
            return;
        }

        m_latestVersion = remoteVersion;
        m_updateUrl = downloadUrl;
        m_updateAvailable = (m_latestVersion != getCurrentVersion());

        emit updateAvailableChanged();
        emit latestVersionChanged();
        emit checkComplete(m_updateAvailable, m_latestVersion);
    });

    networkManager->get(QNetworkRequest(QUrl(serverUrl)));
}

void UpdateManager::downloadUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "download called with no update URL";
        return;
    }
    qInfo(lcUpdate) << "download url=" << m_updateUrl;

    auto* networkManager = new QNetworkAccessManager(this);

    connect(networkManager, &QNetworkAccessManager::finished, this, [this, networkManager](QNetworkReply* reply) {
        networkManager->deleteLater();
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning(lcUpdate) << "Download failed:" << reply->errorString();
            emit errorOccurred(reply->errorString());
            return;
        }

        QString destPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/update.raucb";
        QFile file(destPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            qInfo(lcUpdate) << "Update downloaded successfully to" << destPath;
            emit downloadFinished();
        } else {
            const QString msg = QStringLiteral("Failed to save update bundle to %1").arg(destPath);
            qWarning(lcUpdate) << msg;
            emit errorOccurred(msg);
        }
    });

    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(m_updateUrl)));
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
