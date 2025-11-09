#include "f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp"
#include <QDBusReply>
#include <QUuid>

// Define our logging category
Q_LOGGING_CATEGORY(logWifi, "autoapp.wifi")

// D-Bus constants for NetworkManager
const QString NM_SERVICE = "org.freedesktop.NetworkManager";
const QString NM_PATH = "/org/freedesktop/NetworkManager";
const QString NM_INTERFACE = "org.freedesktop.NetworkManager";
const QString NM_DEVICE_IFACE = "org.freedesktop.NetworkManager.Device";
const QString NM_WIRELESS_IFACE = "org.freedesktop.NetworkManager.Device.Wireless";
const QString NM_AP_IFACE = "org.freedesktop.NetworkManager.AccessPoint";
const QString NM_ACTIVE_CONN_IFACE = "org.freedesktop.NetworkManager.Connection.Active";
const QString NM_SETTINGS_IFACE = "org.freedesktop.NetworkManager.Settings";

/**
 * Constructor: Connect to D-Bus and set up initial signal monitoring.
 */
WifiMonitor::WifiMonitor(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, QObject *parent)
    : QObject(parent),
      m_systemBus(QDBusConnection::systemBus()),
      m_nm(nullptr),
      m_currentMode(f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT),
      m_connected(false),
      m_signalStrength(0),
      configuration_(std::move(configuration)) {

    m_hotspotSsid = configuration_->getSettingByName<QString>("Wireless", "HotspotSSID");
    m_currentMode = configuration_->getSettingByName<f1x::openauto::autoapp::UI::Enum::WirelessType::Value>("Wireless", "Type");
    m_hotspotPassword = configuration_->getSettingByName<QString>("Wireless", "ClientSSID");

    if (!m_systemBus.isConnected()) {
        qCCritical(logWifi) << "Cannot connect to the D-Bus system bus.";
        return;
    }

    // 1. Connect to the main NetworkManager StateChanged signal
    m_systemBus.connect(NM_SERVICE, NM_PATH, NM_INTERFACE, "StateChanged",
                        this, SLOT(onNmStateChanged(quint32)));

    // 2. Create an interface to call methods on NetworkManager
    m_nm = new QDBusInterface(NM_SERVICE, NM_PATH, NM_INTERFACE, m_systemBus, this);
    if (!m_nm->isValid()) {
        qCCritical(logWifi) << "Failed to create D-Bus interface for NetworkManager.";
        return;
    }

    // 3. Find our wlan0 device
    findWifiDevice();

    // 4. Do an initial poll of the current state
    onNmStateChanged(m_nm->property("State").toUInt());
}

WifiMonitor::~WifiMonitor() {
    // Clean up D-Bus connections
}

/**
 * Finds the D-Bus path for the 'wlan0' device and connects to its signals.
 */
void WifiMonitor::findWifiDevice() {
    const QDBusReply<QDBusObjectPath> reply = m_nm->call("GetDeviceByIpIface", "wlan0");
    if (reply.isValid()) {
        m_wifiDevicePath = reply.value().path();
        qCInfo(logWifi) << "Found Wi-Fi device at path:" << m_wifiDevicePath;

        // Connect to this device's StateChanged signal
        m_systemBus.connect(NM_SERVICE, m_wifiDevicePath, NM_DEVICE_IFACE, "StateChanged",
                            this, SLOT(onDeviceStateChanged(quint32,quint32,quint32)));

        // Connect to this device's "ActiveConnection" property changing
        m_systemBus.connect(NM_SERVICE, m_wifiDevicePath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            this, SLOT(onActiveConnectionChanged(QVariantMap)));

        m_systemBus.connect(NM_SERVICE, m_wifiDevicePath, NM_WIRELESS_IFACE, "ScanDone",
                            this, SLOT(onScanDone()));

    } else {
        qCWarning(logWifi) << "Could not find D-Bus path for 'wlan0'.";
    }
}

// --- Property READ functions ---

f1x::openauto::autoapp::UI::Enum::WirelessType::Value WifiMonitor::getCurrentMode() const { return m_currentMode; }
bool WifiMonitor::isConnected() const { return m_connected; }
QString WifiMonitor::getCurrentSsid() const { return m_currentSsid; }
QVariantList WifiMonitor::getAccessPoints() const { return m_accessPoints; }
QString WifiMonitor::getHotspotSsid() const { return m_hotspotSsid; } // TODO: Read from config
QString WifiMonitor::getHotspotPassword() const { return m_hotspotPassword; } // TODO: Read from config
int WifiMonitor::getSignalStrength() const {
    // You would get this from the ActiveConnection or AccessPoint properties
    return m_signalStrength;
}


// --- Q_INVOKABLE Method Implementations ---

/**
 * Q_INVOKABLE: Sets the Wi-Fi mode to Client or Hotspot.
 */
void WifiMonitor::setMode(const f1x::openauto::autoapp::UI::Enum::WirelessType::Value mode) const {
    if (mode == m_currentMode) return;

    // Disconnect any active connection first
    disconnect();

    if (mode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT) {
        qCInfo(logWifi) << "Setting mode to Hotspot (AP)...";

        // This is a complex D-Bus call to create a new AP connection
        // See NetworkManager D-Bus docs for "AddAndActivateConnection"
        // You need to build a QVariantMap describing the AP connection.

        // --- Simplified Example ---
        QVariantMap connectionSettings;
        connectionSettings.insert("connection.type", "802-11-wireless");
        connectionSettings.insert("connection.id", m_hotspotSsid);
        connectionSettings.insert("connection.uuid", QUuid::createUuid().toString());
        connectionSettings.insert("802-11-wireless.mode", "ap");
        connectionSettings.insert("802-11-wireless.ssid", m_hotspotSsid.toUtf8());
        connectionSettings.insert("802-11-wireless-security.key-mgmt", "wpa-psk");
        connectionSettings.insert("802-11-wireless-security.psk", m_hotspotPassword);

        m_nm->call("AddAndActivateConnection", QVariant::fromValue(connectionSettings),
                   QVariant::fromValue(QDBusObjectPath(m_wifiDevicePath)),
                   QVariant::fromValue(QDBusObjectPath("/")));

        // The onDeviceStateChanged slot will update our properties

    } else if (mode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_CLIENT) {
        qCInfo(logWifi) << "Setting mode to Client (Station)...";
        // No action needed here, disconnecting the AP
        // will automatically put the device back in Client mode.
        // We can trigger a scan for the UI.
        scanForNetworks();
    }
}

/**
 * Q_INVOKABLE: Scans for available Wi-Fi networks.
 */
void WifiMonitor::scanForNetworks() const {
    if (m_wifiDevicePath.isEmpty() || m_currentMode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT) {
        qCWarning(logWifi) << "Cannot scan while in Hotspot mode or if device is missing.";
        return;
    }

    QDBusInterface wirelessIface(NM_SERVICE, m_wifiDevicePath, NM_WIRELESS_IFACE, m_systemBus);
    wirelessIface.call("RequestScan", QVariantMap());

}

/**
 * Q_INVOKABLE: Connects to a specific Wi-Fi network.
 */
void WifiMonitor::connectToSsid(const QString &ssid, const QString &password) const {
    if (m_wifiDevicePath.isEmpty() || m_currentMode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT) {
        qCWarning(logWifi) << "Cannot connect while in Hotspot mode.";
        return;
    }

    qCInfo(logWifi) << "Attempting to connect to SSID:" << ssid;

    // This is a simplified connection. For a real implementation, you would
    // find the AccessPoint's D-Bus path from your m_accessPoints list
    // and pass that to AddAndActivateConnection.

    // --- Simplified Example ---
    QVariantMap connectionSettings;
    connectionSettings.insert("connection.type", "802-11-wireless");
    connectionSettings.insert("connection.id", ssid);
    connectionSettings.insert("connection.uuid", QUuid::createUuid().toString());
    connectionSettings.insert("802-11-wireless.ssid", ssid.toUtf8());
    connectionSettings.insert("802-11-wireless-security.key-mgmt", "wpa-psk");
    connectionSettings.insert("802-11-wireless-security.psk", password);

    m_nm->call("AddAndActivateConnection", QVariant::fromValue(connectionSettings),
                   QVariant::fromValue(QDBusObjectPath(m_wifiDevicePath)),
                   QVariant::fromValue(QDBusObjectPath("/")));
}

/**
 * Q_INVOKABLE: Disconnects from the current network (Client or Hotspot).
 */
void WifiMonitor::disconnect() const {
    if (!m_connected || m_wifiDevicePath.isEmpty()) return;

    qCInfo(logWifi) << "Disconnecting active connection...";

    QDBusInterface deviceIface(NM_SERVICE, m_wifiDevicePath, NM_DEVICE_IFACE, m_systemBus);
    deviceIface.call("Disconnect");
}


// --- Private D-Bus Signal Handlers ---

/**
 * SLOT: Called when the *master* NetworkManager state changes.
 */
void WifiMonitor::onNmStateChanged(const quint32 state) {
    // 70 = Connected, 60 = Connecting, 20 = Disconnected, 10 = Asleep
    qCDebug(logWifi) << "NetworkManager state changed:" << state;

    if (state < 70) {
        if (m_connected) {
            m_connected = false;
            m_currentSsid = "";
            m_signalStrength = 0;
            emit connectionChanged();
        }
    }
}

/**
 * SLOT: Called when the *wlan0 device* state changes.
 */
void WifiMonitor::onDeviceStateChanged(const quint32 newState, quint32 oldState, quint32 reason) {
    qCDebug(logWifi) << "Wi-Fi device (" << m_wifiDevicePath << ") state changed to" << newState;

    // 100 = Connected, 80 = Activating, 30 = Disconnected
    if (newState == 100) {
        // Device is active, find out what connection it's using
        const QDBusInterface deviceIface(NM_SERVICE, m_wifiDevicePath, NM_DEVICE_IFACE, m_systemBus);
        updateActiveConnection(deviceIface.property("ActiveConnection").value<QDBusObjectPath>());

    } else if (newState <= 30) {
        // Device is disconnected
        if (m_connected) {
            m_connected = false;
            m_currentSsid = "";
            m_signalStrength = 0;
            m_currentMode = (m_currentMode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT) ? f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT : f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_CLIENT; // Guess
            if (m_currentMode == f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_CLIENT) m_currentMode = f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_CLIENT;

            emit connectionChanged();
            emit currentModeChanged();
        }
    }
}

/**
 * SLOT: Called when the wlan0 device's ActiveConnection property changes.
 */
void WifiMonitor::onActiveConnectionChanged(const QVariantMap &properties) {
    if (properties.contains("ActiveConnection")) {
        updateActiveConnection(properties.value("ActiveConnection").value<QDBusObjectPath>());
    }
}

/**
 * Helper: Gets details for the currently active connection.
 */
void WifiMonitor::updateActiveConnection(const QDBusObjectPath &activeConnPath) {
    if (activeConnPath.path().isEmpty() || activeConnPath.path() == "/") {
        if (m_connected) { // We just got disconnected
             m_connected = false;
             m_currentSsid = "";
             emit connectionChanged();
        }
        return;
    }

    const QDBusInterface activeConnIface(NM_SERVICE, activeConnPath.path(), NM_ACTIVE_CONN_IFACE, m_systemBus);

    // Get the actual "Connection" object path
    const QDBusObjectPath connPath = activeConnIface.property("Connection").value<QDBusObjectPath>();
    QDBusInterface connIface(NM_SERVICE, connPath.path(), NM_SETTINGS_IFACE, m_systemBus);

    const QDBusReply<QVariantMap> reply = connIface.call("GetSettings");
    if (!reply.isValid()) return;

    QVariantMap settings = reply.value();
    const QString type = settings["connection"].toMap()["type"].toString();

    if (type == "802-11-wireless") {
        m_currentMode = f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_CLIENT;
        m_currentSsid = QString::fromUtf8(settings["802-11-wireless"].toMap()["ssid"].toByteArray());
    } else if (type == "ap" || (type == "802-11-wireless" && settings["802-11-wireless"].toMap()["mode"].toString() == "ap")) {
        m_currentMode = f1x::openauto::autoapp::UI::Enum::WirelessType::WIRELESS_HOTSPOT;
        m_currentSsid = QString::fromUtf8(settings["802-11-wireless"].toMap()["ssid"].toByteArray());
    }

    m_connected = true;
    emit connectionChanged();
    emit currentModeChanged();
}

/**
 * SLOT: Called after a scan finishes. We now fetch the results.
 */
void WifiMonitor::onScanDone() {
    qCDebug(logWifi) << "Scan done. Fetching results...";
    m_systemBus.disconnect(NM_SERVICE, m_wifiDevicePath, NM_WIRELESS_IFACE, "ScanDone",
                           this, SLOT(onScanDone()));

    updateAccessPoints();
}

/**
 * Helper: Fetches all APs from NetworkManager
 */
void WifiMonitor::updateAccessPoints() {
    QDBusInterface wirelessIface(NM_SERVICE, m_wifiDevicePath, NM_WIRELESS_IFACE, m_systemBus);
    const QDBusPendingCall asyncCall = wirelessIface.asyncCall("GetAccessPoints");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, &WifiMonitor::onAccessPointsReply);
}

/**
 * SLOT: Called when the list of Access Points is received.
 */
void WifiMonitor::onAccessPointsReply() {
    // 1. Get the watcher that sent the signal
    auto *watcher = qobject_cast<QDBusPendingCallWatcher *>(sender());
    if (!watcher) {
        qCWarning(logWifi) << "Got access points reply from unknown sender";
        return;
    }

    // 2. Get the reply data from the watcher
    QDBusPendingReply<QList<QDBusObjectPath>> reply = *watcher;

    // 3. Process the reply
    if (reply.isError()) {
        qCWarning(logWifi) << "Failed to get access points:" << reply.error().message();
    } else {
        m_accessPoints.clear();
        QList<QDBusObjectPath> apPaths = reply.value();
        
        for (const QDBusObjectPath &apPath : apPaths) {
            QDBusInterface apIface(NM_SERVICE, apPath.path(), NM_AP_IFACE, m_systemBus);
            QVariantMap apData;
            apData.insert("ssid", QString::fromUtf8(apIface.property("Ssid").toByteArray()));
            apData.insert("strength", apIface.property("Strength").toUInt());
            m_accessPoints.append(apData);
        }
        
        emit accessPointsChanged();
    }

    // 4. --- THE FIX ---
    // This is the crucial part. Mark the watcher for deletion.
    watcher->deleteLater();
}

// You still have these stubs from your code, which you can now remove
// void WifiMonitor::onStaConnected(const QString &address, const QString &interface) {}
// void WifiMonitor::onStaDisconnected(const QString &address, const QString &interface) {}
// void WifiMonitor::onStateChanged(int newState, const QString &interface) {}
// void WifiMonitor::updateIcon(const QString &interface, const QString &color) {}