#pragma once

#include <QObject>
#include <QString>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp::UI {

    class UpdateManager : public QObject {
        Q_OBJECT

        Q_PROPERTY(bool updateAvailable READ isUpdateAvailable NOTIFY updateAvailableChanged)
        Q_PROPERTY(QString latestVersion READ getLatestVersion NOTIFY latestVersionChanged)
        Q_PROPERTY(QString currentVersion READ getCurrentVersion CONSTANT)

    public:
        explicit UpdateManager(configuration::IConfiguration::Pointer config,
                               QObject *parent = nullptr);

        bool isUpdateAvailable() const { return m_updateAvailable; }
        QString getLatestVersion() const { return m_latestVersion; }
        QString getCurrentVersion() const;

        Q_INVOKABLE void checkForUpdate();
        Q_INVOKABLE void downloadUpdate();
        Q_INVOKABLE void applyUpdate();

    signals:
        void updateAvailableChanged();
        void latestVersionChanged();
        void checkComplete(bool available, const QString &version);
        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void downloadFinished();
        void installFinished(bool success, const QString &message);
        void errorOccurred(const QString &message);

    private:
        configuration::IConfiguration::Pointer m_config;
        bool    m_updateAvailable{false};
        QString m_latestVersion;
        QString m_updateUrl;
    };

} // namespace
