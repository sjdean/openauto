#pragma once

#include <QObject>
#include <QVariantList>
#include "f1x/openauto/autoapp/UI/Controller/IWiFiController.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/IWiFiMonitor.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
    class WifiViewModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(
            QString selectedInterface READ getSelectedInterface WRITE setSelectedInterface NOTIFY selectedInterfaceChanged)
        Q_PROPERTY(common::Enum::WirelessType::Value mode READ getMode WRITE setMode NOTIFY modeChanged)
        Q_PROPERTY(bool isHotspot READ getIsHotspot NOTIFY modeChanged)
        Q_PROPERTY(bool isEnabled READ getIsEnabled WRITE setIsEnabled NOTIFY isEnabledChanged)
        Q_PROPERTY(QString hotspotSsid READ getHotspotSsid WRITE setHotspotSsid NOTIFY hotspotSsidChanged)
        Q_PROPERTY(QString hotspotPassword READ getHotspotPassword WRITE setHotspotPassword NOTIFY hotspotPasswordChanged)
        Q_PROPERTY(QString clientSsid READ getClientSsid WRITE setClientSsid NOTIFY clientSsidChanged)
        Q_PROPERTY(QString clientPassword READ getClientPassword WRITE setClientPassword NOTIFY clientPasswordChanged)
        Q_PROPERTY(QString currentSsid READ getCurrentSsid NOTIFY currentSsidChanged)
        Q_PROPERTY(int signalStrength READ getSignalStrength NOTIFY signalStrengthChanged)
        Q_PROPERTY(bool connected READ getConnected NOTIFY connectedChanged)
        Q_PROPERTY(QVariantList accessPoints READ getAccessPoints NOTIFY accessPointsChanged)
        Q_PROPERTY(QVariantList availableInterfaces READ getAvailableInterfaces NOTIFY availableInterfacesChanged)

    public:
        explicit WifiViewModel(configuration::IConfiguration::Pointer config,
                                       UI::Controller::IWiFiController* controller,
                                       UI::Monitor::IWiFiMonitor* monitor,
                                       QObject *parent = nullptr);
        // Read
        QString getSelectedInterface() const;

        common::Enum::WirelessType::Value getMode() const;
        bool getIsHotspot() const;
        QString getHotspotSsid() const;
        QString getHotspotPassword() const;
        QString getClientSsid() const;
        QString getClientPassword() const;
        QString getCurrentSsid() const;
        int getSignalStrength() const;
        bool getConnected() const;
        bool getIsEnabled() const;

        QVariantList getAccessPoints() const;
        QVariantList getAvailableInterfaces() const;

        // Write (saves to config + emits)
        Q_INVOKABLE void setSelectedInterface(const QString &iface);
        Q_INVOKABLE void setMode(common::Enum::WirelessType::Value mode);
        Q_INVOKABLE void setHotspotSsid(const QString &ssid);
        Q_INVOKABLE void setHotspotPassword(const QString &password);
        Q_INVOKABLE void setClientSsid(const QString &ssid);
        Q_INVOKABLE void setClientPassword(const QString &password);
        Q_INVOKABLE void setIsEnabled(bool enabled);

        Q_INVOKABLE void doWirelessNetworkScan();
        Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);

    public slots:
        void updateCurrentSsid(const QString &ssid);
        void updateSignalStrength(int strength);
        void updateConnected(bool connected);
        void updateMode(common::Enum::WirelessType::Value mode);
        void updateAccessPoints(const QVariantList &aps);

        // Interface info
        void updateInterface(const QString &macAddress);
        void updateInterfaceUp(bool up);
        void updateCurrentIP(const QString &ip);
        void updateAvailableInterfaces(const QVariantList &interfaces);

    signals:
        void isEnabledChanged();
        void selectedInterfaceChanged();
        void modeChanged();
        void hotspotSsidChanged();
        void hotspotPasswordChanged();
        void clientSsidChanged();
        void clientPasswordChanged();
        void currentSsidChanged();
        void signalStrengthChanged();
        void connectedChanged();
        void accessPointsChanged();
        void scanRequested();
        void interfaceMacChanged();
        void interfaceUpChanged();
        void currentIpChanged();
        void availableInterfacesChanged();

        void connectRequested(const QString &ssid);

    private:
        configuration::IConfiguration::Pointer m_config;
        Controller::IWiFiController* m_wifiController;
        Monitor::IWiFiMonitor* m_monitor;
        common::Enum::WirelessType::Value m_mode;
        QString m_selectedInterface;
        QString m_hotspotSsid;
        QString m_hotspotPassword;
        QString m_clientSsid;
        QString m_clientPassword;
        QString m_currentSsid;
        int m_signalStrength{0};
        bool m_connected{false};
        bool m_isEnabled{false};
        QVariantList m_accessPoints;
        QVariantList m_availableInterfaces;
        QString m_currentIp;
        bool m_interfaceUp;
        QString m_interfaceMac;
    };
} // namespace
