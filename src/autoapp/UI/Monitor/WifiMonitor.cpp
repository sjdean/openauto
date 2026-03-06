#include "f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp"
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#ifdef Q_OS_LINUX
#include <QDBusPendingReply>
#include <QDBusReply>
#endif

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcWifiMonitor, "journeyos.wifi.monitor")

namespace f1x::openauto::autoapp::UI::Monitor {
    using namespace f1x::openauto::common::Enum;

    WifiMonitor::WifiMonitor(configuration::IConfiguration::Pointer config, QObject *parent)
        : QObject(parent)
          , m_config(std::move(config))
          , m_refreshTimer(new QTimer(this)) {
        // === 1. Find current Wi-Fi interface (cross-platform) ===
        QString macAddress = m_config->getSettingByName<QString>("Wireless", "Interface");

        if (!macAddress.isEmpty()) {
            // Find interface by MAC (most reliable)
            for (const QNetworkInterface &i: QNetworkInterface::allInterfaces()) {
                if (i.type() == QNetworkInterface::Wifi &&
                    i.hardwareAddress().compare(macAddress, Qt::CaseInsensitive) == 0) {
                    m_currentInterface = i;
                    break;
                }
            }
        }

        // Fallback: if no MAC saved, auto-detect first Wi-Fi adapter
        if (!m_currentInterface.isValid()) {
            for (const QNetworkInterface &i: QNetworkInterface::allInterfaces()) {
                if (i.type() == QNetworkInterface::Wifi &&
                    (i.flags() & QNetworkInterface::IsUp) &&
                    !i.hardwareAddress().isEmpty()) {
                    m_currentInterface = i;
                    break;
                }
            }
        }

        // Save MAC forever if we have a valid adapter
        if (m_currentInterface.isValid()) {
            QString currentMac = m_currentInterface.hardwareAddress();
            if (macAddress.isEmpty() || macAddress != currentMac) {
                m_config->updateSettingByName<QString>("Wireless", "Interface", currentMac);
                m_config->save();
            }
        }


#ifdef Q_OS_LINUX
        m_nm = new QDBusInterface("org.freedesktop.NetworkManager",
                                  "/org/freedesktop/NetworkManager",
                                  "org.freedesktop.NetworkManager",
                                  m_bus, this);

        if (m_currentInterface.isValid()) {
            findWifiDevice(m_currentInterface.name()); // Still need name for D-Bus
        }
#endif

        // === 2. Start refresh timer (all platforms) ===
        connect(m_refreshTimer, &QTimer::timeout, this, &WifiMonitor::refreshCrossPlatformInfo);
        m_refreshTimer->start(5000);

        // Initial refresh
        refreshCrossPlatformInfo();
        updateInterfaceList();

        emit interfaceChanged(m_currentInterface.hardwareAddress());
    }

    WifiMonitor::~WifiMonitor() {
#ifdef Q_OS_LINUX
        delete m_nm;
#endif
    }

    void WifiMonitor::refreshCrossPlatformInfo() {
        if (!m_currentInterface.isValid()) {
            emit currentSsidChanged(tr("No Wi-Fi adapter"));
            emit signalStrengthChanged(0);
            emit connectedChanged(false);
            emit interfaceUpChanged(false);
            emit currentIpChanged("");
            return;
        }

        // These work perfectly on Linux, macOS, and Windows
        bool isUp = m_currentInterface.flags() & QNetworkInterface::IsUp;
        emit interfaceUpChanged(isUp);
        emit connectedChanged(isUp); // Good enough for now

        updateCurrentIp(); // Shows real IP on all platforms
        updateInterfaceList(); // Shows real interface list + MAC

#ifdef Q_OS_LINUX
        // Linux: Full SSID + signal from NetworkManager D-Bus (your existing code)
        // → Do nothing here, your D-Bus slots already emit currentSsidChanged/signalStrengthChanged

#else
        // macOS + Windows: We don't have SSID/signal yet → show honest placeholder
        emit currentSsidChanged(tr("Not available"));
        emit signalStrengthChanged(0);
#endif
    }

    void WifiMonitor::updateCurrentIp() {
        QString ip;
        for (const auto &entry: m_currentInterface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ip = entry.ip().toString();
                break;
            }
        }
        emit currentIpChanged(ip);
    }

    void WifiMonitor::updateInterfaceList() {
        QVariantList list;

        for (const QNetworkInterface &i: QNetworkInterface::allInterfaces()) {
            if (i.type() != QNetworkInterface::Wifi || i.hardwareAddress().isEmpty()) {
                continue;
            }

            QVariantMap map;
            map["name"] = i.name();
            map["mac"] = i.hardwareAddress();
            map["displayName"] = QString("%1 (%2)").arg(i.name(), i.hardwareAddress());
            map["up"] = i.flags().testFlag(QNetworkInterface::IsUp); // Fixed line
            map["running"] = i.flags().testFlag(QNetworkInterface::IsRunning);

            // Optional: add current IP
            QString ip;
            for (const QNetworkAddressEntry &entry: i.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ip = entry.ip().toString();
                    break;
                }
            }
            map["ip"] = ip;

            list.append(map);
        }

        emit availableInterfacesChanged(list);
    }

    void WifiMonitor::requestScan()
    {
#ifdef Q_OS_LINUX
        if (m_wifiDevicePath.isEmpty()) return;

        QDBusInterface wireless("org.freedesktop.NetworkManager",
                                m_wifiDevicePath,
                                "org.freedesktop.NetworkManager.Device.Wireless",
                                m_bus);

        QVariantMap options; // NM accepts empty options map
        auto *w = new QDBusPendingCallWatcher(wireless.asyncCall("RequestScan", options), this);
        connect(w, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
            if (call->isError())
                qWarning(lcWifiMonitor) << "RequestScan failed:" << call->error().message();
            else
                QTimer::singleShot(2000, this, &WifiMonitor::refreshAccessPoints);
            call->deleteLater();
        });
#endif
    }

#ifdef Q_OS_LINUX
    void WifiMonitor::refreshAccessPoints()
    {
        if (m_wifiDevicePath.isEmpty()) return;

        QDBusInterface wireless("org.freedesktop.NetworkManager",
                                m_wifiDevicePath,
                                "org.freedesktop.NetworkManager.Device.Wireless",
                                m_bus);

        QDBusReply<QList<QDBusObjectPath>> reply = wireless.call("GetAllAccessPoints");
        if (!reply.isValid()) {
            qWarning(lcWifiMonitor) << "GetAllAccessPoints failed:" << reply.error().message();
            return;
        }

        QVariantList apList;
        for (const QDBusObjectPath &apPath : reply.value()) {
            QDBusInterface ap("org.freedesktop.NetworkManager",
                              apPath.path(),
                              "org.freedesktop.NetworkManager.AccessPoint",
                              m_bus);

            QByteArray ssidBytes = ap.property("Ssid").toByteArray();
            QString ssid = QString::fromUtf8(ssidBytes);
            if (ssid.isEmpty()) continue;

            int strength = ap.property("Strength").toInt();
            bool secured = (ap.property("WpaFlags").toUInt() | ap.property("RsnFlags").toUInt()) > 0;

            QVariantMap apMap;
            apMap["ssid"] = ssid;
            apMap["strength"] = strength;
            apMap["secured"] = secured;
            apList.append(apMap);
        }

        std::sort(apList.begin(), apList.end(), [](const QVariant &a, const QVariant &b) {
            return a.toMap()["strength"].toInt() > b.toMap()["strength"].toInt();
        });

        emit accessPointsChanged(apList);
    }
#endif

    // ————————————————————
    // Linux-only D-Bus code
    // ————————————————————

#ifdef Q_OS_LINUX

    void WifiMonitor::findWifiDevice(const QString &ifaceName) {
        if (ifaceName.isEmpty()) return;

        QDBusInterface nm("org.freedesktop.NetworkManager",
                          "/org/freedesktop/NetworkManager",
                          "org.freedesktop.NetworkManager", m_bus);

        QDBusPendingReply<QDBusObjectPath> reply = nm.asyncCall("GetDeviceByIpIface", ifaceName);
        auto *watcher = new QDBusPendingCallWatcher(reply, this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QDBusObjectPath> r = *w;
            if (r.isError()) {
                qWarning(lcWifiMonitor) << "Wi-Fi device not found:" << r.error().message();
                w->deleteLater();
                return;
            }

            m_wifiDevicePath = r.value().path();
            qInfo(lcWifiMonitor) << "Monitoring Wi-Fi device:" << m_wifiDevicePath;

            // Watch state changes (connected/disconnected)
            m_bus.connect("org.freedesktop.NetworkManager",
                          m_wifiDevicePath,
                          "org.freedesktop.NetworkManager.Device",
                          "StateChanged",
                          this, SLOT(onDeviceStateChanged(quint32, quint32, quint32)));

            // Watch active connection changes (SSID, AP mode, etc.)
            m_bus.connect("org.freedesktop.NetworkManager",
                          m_wifiDevicePath,
                          "org.freedesktop.DBus.Properties",
                          "PropertiesChanged",
                          this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

            // Initial state
            refreshLinuxStatus();

            w->deleteLater();
        });
    }

    void WifiMonitor::onDeviceStateChanged(quint32 newState, quint32 /*oldState*/, quint32 /*reason*/) {
        // NetworkManager state codes: https://networkmanager.dev/docs/api/latest/nm-dbus-types.html#NMDeviceState
        const bool connected = (newState >= 100); // 100 = NM_DEVICE_STATE_ACTIVATED
        emit connectedChanged(connected);

        if (connected) {
            refreshLinuxStatus(); // Get SSID, signal, mode
        } else {
            emit currentSsidChanged("");
            emit signalStrengthChanged(0);
            emit modeChanged(WirelessType::WIRELESS_CLIENT);
        }
    }

    void WifiMonitor::onPropertiesChanged(const QString &interfaceName,
                                          const QVariantMap &changed,
                                          const QStringList & /*invalidated*/) {
        if (interfaceName != "org.freedesktop.NetworkManager.Device") return;

        if (changed.contains("ActiveConnection") || changed.contains("State")) {
            refreshLinuxStatus();
        }
    }

    void WifiMonitor::refreshLinuxStatus() {
        if (m_wifiDevicePath.isEmpty()) return;

        QDBusInterface device("org.freedesktop.NetworkManager",
                              m_wifiDevicePath,
                              "org.freedesktop.NetworkManager.Device",
                              m_bus);

        QDBusObjectPath activeConnPath = device.property("ActiveConnection").value<QDBusObjectPath>();
        if (activeConnPath.path() == "/" || activeConnPath.path().isEmpty()) {
            emit currentSsidChanged("");
            emit modeChanged(WirelessType::WIRELESS_CLIENT);
            return;
        }

        QDBusInterface activeConn("org.freedesktop.NetworkManager",
                                  activeConnPath.path(),
                                  "org.freedesktop.NetworkManager.Connection.Active",
                                  m_bus);

        QDBusObjectPath connSettingsPath = activeConn.property("Connection").value<QDBusObjectPath>();
        QDBusObjectPath specificObject = activeConn.property("SpecificObject").value<QDBusObjectPath>();

        // Get connection settings (SSID, mode)
        QDBusInterface settings("org.freedesktop.NetworkManager",
                                connSettingsPath.path(),
                                "org.freedesktop.NetworkManager.Settings.Connection",
                                m_bus);

        QDBusPendingReply<QVariantMap> reply = settings.asyncCall("GetSettings");
        auto *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, specificObject](QDBusPendingCallWatcher *w) {
            if (w->isError()) {
                qWarning(lcWifiMonitor) << "Failed to get connection settings:" << w->error().message();
                w->deleteLater();
                return;
            }

            QVariantMap settings = w->reply().arguments().first().toMap();
            QVariantMap wireless = settings.value("802-11-wireless").toMap();
            QByteArray ssidBytes = wireless.value("ssid").toByteArray();
            QString ssid = QString::fromUtf8(ssidBytes);
            QString mode = wireless.value("mode").toString(); // "infrastructure" or "ap"

            emit currentSsidChanged(ssid);
            emit modeChanged(mode == "ap" ? WirelessType::WIRELESS_HOTSPOT : WirelessType::WIRELESS_CLIENT);

            // Signal strength from current AccessPoint
            if (!specificObject.path().isEmpty() && specificObject.path() != "/") {
                QDBusInterface ap("org.freedesktop.NetworkManager",
                                  specificObject.path(),
                                  "org.freedesktop.NetworkManager.AccessPoint",
                                  m_bus);
                quint8 strength = ap.property("Strength").value<quint8>();
                emit signalStrengthChanged(static_cast<int>(strength));
            } else {
                emit signalStrengthChanged(0);
            }

            w->deleteLater();
        });
    }

#endif // Q_OS_LINUX
}
