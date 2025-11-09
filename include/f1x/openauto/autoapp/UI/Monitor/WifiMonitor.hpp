#pragma once

#include <QObject>
#include <QVariantList>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

#include "f1x/openauto/autoapp/UI/Enum/WirelessType.hpp"

// Define a logging category for this class
Q_DECLARE_LOGGING_CATEGORY(logWifi)

class WifiMonitor : public QObject {
    Q_OBJECT

    // 1. Define your QML-accessible properties
    //----------------------------------------------------
    Q_ENUMS(WifiMode)

    Q_PROPERTY(f1x::openauto::autoapp::UI::Enum::WirelessType::Value currentMode READ getCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString currentSsid READ getCurrentSsid NOTIFY connectionChanged)
    Q_PROPERTY(int signalStrength READ getSignalStrength NOTIFY signalStrengthChanged) // 0-100
    Q_PROPERTY(QVariantList accessPoints READ getAccessPoints NOTIFY accessPointsChanged)

    // Properties for Hotspot Mode
    Q_PROPERTY(QString hotspotSsid READ getHotspotSsid NOTIFY hotspotConfigChanged)
    Q_PROPERTY(QString hotspotPassword READ getHotspotPassword NOTIFY hotspotConfigChanged)

public:

    explicit WifiMonitor(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, QObject *parent = nullptr);
    virtual ~WifiMonitor();

    // 2. Add READ functions for your properties
    //----------------------------------------------------
    f1x::openauto::autoapp::UI::Enum::WirelessType::Value getCurrentMode() const;
    bool isConnected() const;
    QString getCurrentSsid() const;
    int getSignalStrength() const;
    QVariantList getAccessPoints() const;

    QString getHotspotSsid() const;
    QString getHotspotPassword() const;

public slots:
    // 3. Add Q_INVOKABLE methods for QML to call
    //----------------------------------------------------
    Q_INVOKABLE void setMode(f1x::openauto::autoapp::UI::Enum::WirelessType ::Value mode) const;
    Q_INVOKABLE void scanForNetworks() const;
    Q_INVOKABLE void connectToSsid(const QString &ssid, const QString &password) const;
    Q_INVOKABLE void disconnect() const;

signals:
    // 4. Add NOTIFY signals for your properties
    //----------------------------------------------------
    void currentModeChanged();
    void connectionChanged();
    void signalStrengthChanged();
    void accessPointsChanged();
    void hotspotConfigChanged();
    void connectionError(const QString &error); // For QML to show errors

private slots:
    // 5. Private slots to handle D-Bus signals
    //----------------------------------------------------
    void onNmStateChanged(quint32 state);
    void onDeviceStateChanged(quint32 newState, quint32 oldState, quint32 reason);
    void onScanDone();
    void onAccessPointsReply();
    void onActiveConnectionChanged(const QVariantMap &properties);

private:
    void findWifiDevice();
    void updateActiveConnection(const QDBusObjectPath &activeConnPath);
    void updateAccessPoints();

    // 6. Member variables
    //----------------------------------------------------
    QDBusConnection m_systemBus;
    QDBusInterface *m_nm; // Main NetworkManager interface
    QString m_wifiDevicePath; // e.g., /org/freedesktop/NetworkManager/Devices/3

    // Property storage
    f1x::openauto::autoapp::UI::Enum::WirelessType::Value m_currentMode;
    bool m_connected;
    QString m_currentSsid;
    int m_signalStrength;
    QVariantList m_accessPoints;

    QString m_hotspotSsid = "JourneyOS-Hotspot";
    QString m_hotspotPassword = "password123";

    f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;
};