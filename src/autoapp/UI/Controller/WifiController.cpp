#include "f1x/openauto/autoapp/UI/Controller/WifiController.hpp"
#include <QTimer>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcWifi, "journeyos.wifi.controller")

namespace f1x::openauto::autoapp::UI::Controller {

WifiController::WifiController(configuration::Configuration::Pointer config, QObject* parent)
    : QObject(parent)
    , m_config(std::move(config))
{
    applyAllSettings();
}

void WifiController::applyAllSettings()
{
#ifdef Q_OS_LINUX
    const QString iface = m_config->getSettingByName<QString>("Wireless", "Interface", "wlan0");
    const auto mode = m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", WirelessType::WIRELESS_CLIENT);

    setInterface(iface);
    setMode(mode);
#endif
}

void WifiController::setInterface(const QString& ifaceName)
{
#ifdef Q_OS_LINUX
    if (ifaceName.isEmpty()) return;
    m_currentIface = ifaceName;

    QDBusInterface nm("org.freedesktop.NetworkManager",
                      "/org/freedesktop/NetworkManager",
                      "org.freedesktop.NetworkManager", m_bus);

    auto* watcher = new QDBusPendingCallWatcher(nm.asyncCall("GetDeviceByIpIface", ifaceName), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* w) {
        if (w->isError()) {
            qWarning(lcWifi) << "Interface not found:" << w->error().message();
        } else {
            m_wifiDevicePath = w->reply().argumentAt<0>().value<QDBusObjectPath>().path();
            qInfo(lcWifi) << "Wi-Fi device path resolved:" << m_wifiDevicePath;
        }
        w->deleteLater();
    });
#endif
}

void WifiController::setMode(common::Enum::WirelessType::Value mode)
{
#ifdef Q_OS_LINUX
    if (mode == WirelessType::WIRELESS_HOTSPOT) {
        const QString ssid = m_config->getSettingByName<QString>("Wireless", "HotspotSSID", "MyCarHotspot");
        const QString pass = m_config->getSettingByName<QString>("Wireless", "HotspotPassword", "12345678");
        enableHotspotImpl(ssid, pass);
    } else {
        disconnect(); // fall back to client mode
    }
#endif
}

void WifiController::setHotspotCredentials(const QString& ssid, const QString& password)
{
#ifdef Q_OS_LINUX
    if (m_currentIface.isEmpty()) return;
    m_config->setSetting("Wireless", "HotspotSSID", ssid);
    m_config->setSetting("Wireless", "HotspotPassword", password);
    m_config->setSetting("Wireless", "Type", WirelessType::WIRELESS_HOTSPOT);
    m_config->save();

    enableHotspotImpl(ssid, password);
#endif
}

void WifiController::setWirelessCredentials(const QString& ssid, const QString& password)
{
#ifdef Q_OS_LINUX
    connectToWifiImpl(ssid, password);
#endif
}

#ifdef Q_OS_LINUX
void WifiController::enableHotspotImpl(const QString& ssid, const QString& pass)
{
    if (m_wifiDevicePath.isEmpty()) {
        emit errorOccurred(tr("Wi-Fi interface not ready"));
        return;
    }

    QVariantMap connection, wifi, wifiSec, ipv4;
    connection["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection["id"] = ssid;
    connection["type"] = "802-11-wireless";
    connection["interface-name"] = m_currentIface;

    wifi["ssid"] = ssid.toUtf8();
    wifi["mode"] = "ap";

    wifiSec["key-mgmt"] = "wpa-psk";
    wifiSec["psk"] = pass;

    ipv4["method"] = "shared"; // NAT + 10.42.0.1

    const QVariantMap maps[] = { connection, wifi, wifiSec, ipv4 };

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        "AddAndActivateConnection");

    msg << maps[0] << maps[1] << maps[2] << maps[3]
        << QDBusObjectPath(m_wifiDevicePath)
        << QDBusObjectPath("/");

    auto* w = new QDBusPendingCallWatcher(m_bus.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [ssid](QDBusPendingCallWatcher* call){
        if (call->isError())
            qCritical(lcWifi) << "Hotspot failed:" << call->error().message();
        else
            qInfo(lcWifi) << "Hotspot started:" << ssid;
        call->deleteLater();
    });
}
#endif

void WifiController::scan()
{
#ifdef Q_OS_LINUX
    if (m_wifiDevicePath.isEmpty()) {
        emit errorOccurred(tr("Wi-Fi interface not ready"));
        return;
    }

    QDBusInterface wireless("org.freedesktop.NetworkManager",
                            m_wifiDevicePath,
                            "org.freedesktop.NetworkManager.Device.Wireless",
                            m_bus);

    QVariantMap options; // empty options accepted by NM
    auto* watcher = new QDBusPendingCallWatcher(wireless.asyncCall("RequestScan", options), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* w) {
        if (w->isError())
            emit errorOccurred(tr("Scan failed: %1").arg(w->error().message()));
        else
            emit statusChanged(tr("Scan complete"));
        w->deleteLater();
    });
#endif
}

void WifiController::disconnect()
{
#ifdef Q_OS_LINUX
    if (m_wifiDevicePath.isEmpty()) return;

    QDBusInterface device("org.freedesktop.NetworkManager",
                          m_wifiDevicePath,
                          "org.freedesktop.NetworkManager.Device",
                          m_bus);

    QDBusPendingCall async = device.asyncCall("Disconnect");
    auto* watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* w) {
        if (w->isError())
            qWarning(lcWifi) << "Disconnect failed:" << w->error().message();
        else
            qInfo(lcWifi) << "Disconnected from Wi-Fi";
        w->deleteLater();
    });
#endif
}

void WifiController::connectToNetwork(const QString& ssid, const QString& password)
{
#ifdef Q_OS_LINUX
    connectToWifiImpl(ssid, password);
#else
    Q_UNUSED(ssid)
    Q_UNUSED(password)
#endif
}

#ifdef Q_OS_LINUX
void WifiController::connectToWifiImpl(const QString& ssid, const QString& password)
{
    if (m_wifiDevicePath.isEmpty()) {
        emit errorOccurred(tr("Wi-Fi interface not ready"));
        return;
    }

    QVariantMap connection, wifi, wifiSec, ipv4;
    connection["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection["id"] = ssid;
    connection["type"] = "802-11-wireless";
    connection["interface-name"] = m_currentIface;

    wifi["ssid"] = ssid.toUtf8();
    wifi["mode"] = "infrastructure";

    ipv4["method"] = "auto"; // DHCP

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        "AddAndActivateConnection");

    if (!password.isEmpty()) {
        wifiSec["key-mgmt"] = "wpa-psk";
        wifiSec["psk"] = password;
        msg << connection << wifi << wifiSec << ipv4
            << QDBusObjectPath(m_wifiDevicePath) << QDBusObjectPath("/");
    } else {
        msg << connection << wifi << QVariantMap() << ipv4
            << QDBusObjectPath(m_wifiDevicePath) << QDBusObjectPath("/");
    }

    auto* w = new QDBusPendingCallWatcher(m_bus.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [this, ssid](QDBusPendingCallWatcher* call) {
        if (call->isError()) {
            qCritical(lcWifi) << "Connect failed:" << call->error().message();
            emit errorOccurred(tr("Connect failed: %1").arg(call->error().message()));
        } else {
            qInfo(lcWifi) << "Connecting to:" << ssid;
            emit statusChanged(tr("Connecting to %1").arg(ssid));
        }
        call->deleteLater();
    });
}
#endif

} // namespace