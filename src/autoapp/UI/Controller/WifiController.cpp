#include "f1x/openauto/autoapp/UI/Controller/WifiController.hpp"
#include <QTimer>
#include <qloggingcategory.h>
#ifdef Q_OS_LINUX
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusMetaType>
#endif
Q_LOGGING_CATEGORY(lcWifi, "journeyos.wifi.controller")

namespace f1x::openauto::autoapp::UI::Controller {

WifiController::WifiController(configuration::Configuration::Pointer config, QObject* parent)
    : IWiFiController(parent)
    , m_config(std::move(config))
{
#ifdef Q_OS_LINUX
    qDBusRegisterMetaType<NMConnectionSettings>();
#endif
    applyAllSettings();
}

void WifiController::applyAllSettings()
{
#ifdef Q_OS_LINUX
    const QString iface = m_config->getSettingByName<QString>("Wireless", "Interface", "wlan0");
    const auto mode = m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", common::Enum::WirelessType::WIRELESS_CLIENT);

    setInterface(iface);
    setMode(mode);
#endif
}

void WifiController::setInterface(const QString& ifaceOrMac)
{
#ifdef Q_OS_LINUX
    if (ifaceOrMac.isEmpty()) return;

    // If it looks like a MAC address, resolve to the interface name that NM needs.
    QString ifaceName = ifaceOrMac;
    if (ifaceOrMac.contains(QChar(':'))) {
        for (const QNetworkInterface &i : QNetworkInterface::allInterfaces()) {
            if (i.hardwareAddress().compare(ifaceOrMac, Qt::CaseInsensitive) == 0) {
                ifaceName = i.name();
                break;
            }
        }
    }

    m_currentIface = ifaceName;

    QDBusInterface nm("org.freedesktop.NetworkManager",
                      "/org/freedesktop/NetworkManager",
                      "org.freedesktop.NetworkManager", m_bus);

    auto* watcher = new QDBusPendingCallWatcher(nm.asyncCall("GetDeviceByIpIface", ifaceName), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* w) {
        if (w->isError()) {
            qWarning(lcWifi) << "Interface not found:" << w->error().message();
        } else {
            m_wifiDevicePath = w->reply().arguments().at(0).value<QDBusObjectPath>().path();
            qInfo(lcWifi) << "Wi-Fi device path resolved:" << m_wifiDevicePath;
        }
        w->deleteLater();
    });
#endif
}

void WifiController::setMode(common::Enum::WirelessType::Value mode)
{
#ifdef Q_OS_LINUX
    // Always tear down the current connection before switching modes.
    disconnect();

    if (mode == common::Enum::WirelessType::WIRELESS_HOTSPOT) {
        const QString ssid = m_config->getSettingByName<QString>("Wireless", "HotspotSSID", "MyCarHotspot");
        const QString pass = m_config->getSettingByName<QString>("Wireless", "HotspotPassword", "12345678");
        enableHotspotImpl(ssid, pass);
    } else {
        // Reconnect to saved client network if credentials exist.
        const QString ssid = m_config->getSettingByName<QString>("Wireless", "ClientSSID");
        const QString pass = m_config->getSettingByName<QString>("Wireless", "ClientPassword");
        if (!ssid.isEmpty()) {
            connectToWifiImpl(ssid, pass);
        }
    }
#endif
}

void WifiController::setHotspotCredentials(const QString& ssid, const QString& password)
{
#ifdef Q_OS_LINUX
    if (m_currentIface.isEmpty()) return;
    m_config->updateSettingByName<QString>("Wireless", "HotspotSSID", ssid);
    m_config->updateSettingByName<QString>("Wireless", "HotspotPassword", password);
    m_config->updateSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", common::Enum::WirelessType::WIRELESS_HOTSPOT);
    m_config->save();

    enableHotspotImpl(ssid, password);
#endif
}

void WifiController::setWirelessCredentials(const QString& ssid, const QString& password)
{
    // Credentials are already saved to config by the ViewModel.
    // Explicit connection is triggered only via connectToNetwork().
    Q_UNUSED(ssid)
    Q_UNUSED(password)
}

#ifdef Q_OS_LINUX
void WifiController::enableHotspotImpl(const QString& ssid, const QString& pass)
{
    if (m_wifiDevicePath.isEmpty()) {
        emit errorOccurred(tr("Wi-Fi interface not ready"));
        return;
    }

    // NM AddAndActivateConnection expects a{sa{sv}} as a single argument.
    QVariantMap connection;
    connection["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection["id"]   = ssid;
    connection["type"] = "802-11-wireless";
    connection["interface-name"] = m_currentIface;

    QVariantMap wifi;
    wifi["ssid"] = ssid.toUtf8();
    wifi["mode"] = "ap";

    QVariantMap wifiSec;
    wifiSec["key-mgmt"] = "wpa-psk";
    wifiSec["psk"]      = pass;

    QVariantMap ipv4;
    ipv4["method"] = "shared"; // NAT + 10.42.0.1/24

    NMConnectionSettings settings;
    settings["connection"]              = connection;
    settings["802-11-wireless"]         = wifi;
    settings["802-11-wireless-security"] = wifiSec;
    settings["ipv4"]                    = ipv4;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        "AddAndActivateConnection");

    msg << QVariant::fromValue(settings)
        << QVariant::fromValue(QDBusObjectPath(m_wifiDevicePath))
        << QVariant::fromValue(QDBusObjectPath("/"));

    auto* w = new QDBusPendingCallWatcher(m_bus.asyncCall(msg), this);
    connect(w, &QDBusPendingCallWatcher::finished, this, [ssid](QDBusPendingCallWatcher* call) {
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

    // NM AddAndActivateConnection expects a{sa{sv}} as a single argument.
    QVariantMap connection;
    connection["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection["id"]   = ssid;
    connection["type"] = "802-11-wireless";
    connection["interface-name"] = m_currentIface;

    QVariantMap wifi;
    wifi["ssid"] = ssid.toUtf8();
    wifi["mode"] = "infrastructure";

    QVariantMap ipv4;
    ipv4["method"] = "auto"; // DHCP

    NMConnectionSettings settings;
    settings["connection"]      = connection;
    settings["802-11-wireless"] = wifi;
    settings["ipv4"]            = ipv4;

    if (!password.isEmpty()) {
        QVariantMap wifiSec;
        wifiSec["key-mgmt"] = "wpa-psk";
        wifiSec["psk"]      = password;
        settings["802-11-wireless-security"] = wifiSec;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        "AddAndActivateConnection");

    msg << QVariant::fromValue(settings)
        << QVariant::fromValue(QDBusObjectPath(m_wifiDevicePath))
        << QVariant::fromValue(QDBusObjectPath("/"));

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