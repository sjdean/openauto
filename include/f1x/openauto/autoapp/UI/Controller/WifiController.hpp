#pragma once
#include <QObject>
#include <QString>
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"

namespace f1x::openauto::autoapp::UI::Controller {

    class WifiController : public QObject {
        Q_OBJECT

    public:
        explicit WifiController(configuration::Configuration::Pointer config,
                                QObject *parent = nullptr);

        // Public API used by QML/Settings
        Q_INVOKABLE void applyAllSettings();           // Called on startup
        Q_INVOKABLE void setInterface(const QString& ifaceName);
        Q_INVOKABLE void setMode(common::Enum::WirelessType::Value mode);
        Q_INVOKABLE void setHotspot(const QString& ssid, const QString& password);
        Q_INVOKABLE void connectToWifi(const QString& ssid, const QString& password);
        Q_INVOKABLE void scan();
        Q_INVOKABLE void disconnect();

        void connectToWifiImpl(const QString &ssid, const QString &password);

    signals:
            void errorOccurred(const QString& message);
        void statusChanged(const QString& status);

    private:
#ifdef Q_OS_LINUX
    private: // Linux-only implementation details
        void enableHotspotImpl(const QString& ssid, const QString& password);
        void connectToWifiImpl(const QString& ssid, const QString& password);

        QString m_currentIface;
        QString m_wifiDevicePath;
        QDBusConnection m_bus = QDBusConnection::systemBus();
#endif

        configuration::Configuration::Pointer m_config;
    };

} // namespace