#pragma once
#include <QObject>
#include <QString>
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"

namespace f1x::openauto::autoapp::UI::Controller {
    class WifiController final : public QObject {

    public:
        explicit WifiController(configuration::Configuration::Pointer config,
                                QObject *parent = nullptr);

        // Public API used by QML/Settings
        void setInterface(const QString &ifaceName);
        void setMode(common::Enum::WirelessType::Value mode);
        void setHotspotCredentials(const QString &ssid, const QString &password);
        void setWirelessCredentials(const QString &ssid, const QString &password);
        void scan();
        void disconnect();
        void connectToWifiImpl(const QString &ssid, const QString &password);

    private:
#ifdef Q_OS_LINUX
    private: // Linux-only implementation details
        void enableHotspotImpl(const QString &ssid, const QString &password);
        void connectToWifiImpl(const QString &ssid, const QString &password);

        QString m_currentIface;
        QString m_wifiDevicePath;
        QDBusConnection m_bus = QDBusConnection::systemBus();
#endif
        void applyAllSettings();
        configuration::Configuration::Pointer m_config;
    };
} // namespace
