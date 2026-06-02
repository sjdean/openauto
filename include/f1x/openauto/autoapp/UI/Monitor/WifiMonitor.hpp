// f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp
#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkInterface>
#include <QVariantList>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/IWiFiMonitor.hpp"

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
    class WifiMonitor : public IWiFiMonitor {
        Q_OBJECT

    public:
        explicit WifiMonitor(configuration::IConfiguration::Pointer config, QObject *parent = nullptr);

        ~WifiMonitor() override;

    public slots:
        void requestScan();

    private slots:
        void refreshCrossPlatformInfo();

#ifdef Q_OS_LINUX
        void findWifiDevice(const QString &ifaceName);
        void onDeviceStateChanged(quint32 newState, quint32 oldState, quint32 reason);
        void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changed,
                                 const QStringList &invalidated);
#endif

    private:
        void updateInterfaceList();
        void updateCurrentIp();

#ifdef Q_OS_LINUX
        void refreshLinuxStatus();
        void refreshAccessPoints();
#endif

        configuration::IConfiguration::Pointer m_config;

        QTimer *m_refreshTimer = nullptr;
        QNetworkInterface m_currentInterface;

#ifdef Q_OS_LINUX
        QDBusConnection m_bus = QDBusConnection::systemBus();
        QDBusInterface *m_nm = nullptr;
        QString m_wifiDevicePath;
#endif
    };
} // namespace
