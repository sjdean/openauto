#pragma once
#include <QObject>
#include <QString>
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"
#include "f1x/openauto/autoapp/UI/Controller/IWiFiController.hpp"

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QNetworkInterface>
#include <QUuid>
#endif

// NM AddAndActivateConnection expects a{sa{sv}} — one nested dict, not separate maps
using NMConnectionSettings = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(NMConnectionSettings)

namespace f1x::openauto::autoapp::UI::Controller {
    class WifiController : public IWiFiController {
        Q_OBJECT

    public:
        explicit WifiController(configuration::Configuration::Pointer config,
                                QObject *parent = nullptr);

        void setInterface(const QString &ifaceName);
        void setMode(common::Enum::WirelessType::Value mode);
        void setHotspotCredentials(const QString &ssid, const QString &password);
        void setWirelessCredentials(const QString &ssid, const QString &password);
        void scan();
        void disconnect();
        void connectToNetwork(const QString &ssid, const QString &password);

    signals:
        void errorOccurred(const QString &message);
        void statusChanged(const QString &message);

    private:
        void applyAllSettings();
        configuration::Configuration::Pointer m_config;

#ifdef Q_OS_LINUX
        void enableHotspotImpl(const QString &ssid, const QString &password);
        void connectToWifiImpl(const QString &ssid, const QString &password);

        QString m_currentIface;
        QString m_wifiDevicePath;
        QDBusConnection m_bus = QDBusConnection::systemBus();
#endif
    };
} // namespace