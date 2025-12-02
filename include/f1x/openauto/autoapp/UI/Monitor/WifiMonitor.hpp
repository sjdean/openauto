// f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp
#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkInterface>
#include <QVariantList>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#endif

//#ifdef Q_OS_MACOS
//#include <CoreWLAN/CoreWLAN.h>
//#endif

namespace f1x::openauto::autoapp::UI::Monitor {

    class WifiMonitor : public QObject
    {
        Q_OBJECT

    public:
        explicit WifiMonitor(configuration::IConfiguration::Pointer config, QObject* parent = nullptr);
        ~WifiMonitor() override;

        signals:
            // Real-time status
            void currentSsidChanged(const QString& ssid);
        void signalStrengthChanged(int strength);        // 0–100
        void connectedChanged(bool connected);
        void modeChanged(common::Enum::WirelessType::Value mode);

        // Interface info
        void interfaceChanged(const QString& macAddress);
        void interfaceUpChanged(bool up);
        void currentIpChanged(const QString& ip);
        void availableInterfacesChanged(const QVariantList& interfaces);

        // Scan results (Linux only for now)
        void accessPointsChanged(const QVariantList& aps);

    private slots:
        void refreshCrossPlatformInfo();

#ifdef Q_OS_LINUX
        void findWifiDevice(const QString& ifaceName);
        void onDeviceStateChanged(quint32 newState, quint32 oldState, quint32 reason);
        void onActiveConnectionChanged(const QVariantMap& changed);
        void updateActiveConnection(const QDBusObjectPath& path);
#endif

    private:
        void updateInterfaceList();
        void updateCurrentIp();

        configuration::IConfiguration::Pointer m_config;

        QTimer* m_refreshTimer = nullptr;
        QNetworkInterface m_currentInterface;

#ifdef Q_OS_LINUX
        QDBusConnection m_bus = QDBusConnection::systemBus();
        QDBusInterface* m_nm = nullptr;
        QString m_wifiDevicePath;
#endif
    };

} // namespace