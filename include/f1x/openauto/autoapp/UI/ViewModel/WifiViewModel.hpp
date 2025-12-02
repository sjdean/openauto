#pragma once
#include <QObject>
#include <QVariantList>
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
    class WifiViewModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(
            QString selectedInterface READ selectedInterface WRITE setSelectedInterface NOTIFY selectedInterfaceChanged)
        Q_PROPERTY(common::Enum::WirelessType::Value mode READ mode WRITE setMode NOTIFY modeChanged)
        Q_PROPERTY(bool isHotspot READ isHotspot NOTIFY modeChanged)
        Q_PROPERTY(QString hotspotSsid READ hotspotSsid WRITE setHotspotSsid NOTIFY hotspotSsidChanged)
        Q_PROPERTY(QString hotspotPassword READ hotspotPassword WRITE setHotspotPassword NOTIFY hotspotPasswordChanged)
        Q_PROPERTY(QString clientSsid READ clientSsid WRITE setClientSsid NOTIFY clientSsidChanged)
        Q_PROPERTY(QString clientPassword READ clientPassword WRITE setClientPassword NOTIFY clientPasswordChanged)
        Q_PROPERTY(QString currentSsid READ currentSsid NOTIFY currentSsidChanged)
        Q_PROPERTY(int signalStrength READ signalStrength NOTIFY signalStrengthChanged)
        Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
        Q_PROPERTY(QVariantList accessPoints READ accessPoints NOTIFY accessPointsChanged)

    public:
        explicit WifiViewModel(configuration::IConfiguration::Pointer config, QObject *parent = nullptr);

        // Read
        QString selectedInterface() const;

        common::Enum::WirelessType::Value mode() const;

        bool isHotspot() const;

        QString hotspotSsid() const;

        QString hotspotPassword() const;

        QString clientSsid() const;

        QString clientPassword() const;

        QString currentSsid() const;

        int signalStrength() const;

        bool connected() const;

        QVariantList accessPoints() const;

        // Write (saves to config + emits)
        Q_INVOKABLE void setSelectedInterface(const QString &iface);

        Q_INVOKABLE void setMode(common::Enum::WirelessType::Value mode);

        Q_INVOKABLE void setHotspotSsid(const QString &ssid);

        Q_INVOKABLE void setHotspotPassword(const QString &pass);

        Q_INVOKABLE void setClientSsid(const QString &ssid);

        Q_INVOKABLE void setClientPassword(const QString &pass);

        Q_INVOKABLE void requestScan();

        Q_INVOKABLE void connectToAp(const QString &ssid);

        // Called from WifiMonitor
        void updateCurrentSsid(const QString &ssid);

        void updateSignalStrength(int strength);

        void updateConnected(bool connected);

        void updateMode(common::Enum::WirelessType::Value mode);

        void updateAccessPoints(const QVariantList &aps);

    signals:
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

        void connectRequested(const QString &ssid);

    private:
        configuration::IConfiguration::Pointer m_config;

        QString m_selectedInterface;
        QString m_hotspotSsid;
        QString m_hotspotPassword;
        QString m_clientSsid;
        QString m_clientPassword;
        QString m_currentSsid;
        int m_signalStrength{0};
        bool m_connected{false};
        QVariantList m_accessPoints;
    };
} // namespace
