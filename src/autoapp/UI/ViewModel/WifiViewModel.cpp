// WifiViewModel.cpp
#include "f1x/openauto/autoapp/UI/ViewModel/WifiViewModel.hpp"
#include <QDebug>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/UI/Controller/WifiController.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVmWifi, "journeyos.wifi")

namespace f1x::openauto::autoapp::UI::ViewModel {

    // 1. Accept Pointer in Constructor
    WifiViewModel::WifiViewModel(configuration::IConfiguration::Pointer config, UI::Controller::WifiController* controller, QObject *parent)
        : QObject(parent), m_config(std::move(config)), m_wifiController(controller)
    {
        // Load from config
        m_selectedInterface = m_config->getSettingByName<QString>("Wireless", "Interface");
        m_hotspotSsid       = m_config->getSettingByName<QString>("Wireless", "HotspotSSID");
        m_hotspotPassword   = m_config->getSettingByName<QString>("Wireless", "HotspotPassword");
        m_clientSsid        = m_config->getSettingByName<QString>("Wireless", "ClientSSID");
        m_clientPassword    = m_config->getSettingByName<QString>("Wireless", "ClientPassword");
        m_isEnabled         = m_config->getSettingByName<bool>("Wireless", "Enabled");
        auto modeVal        = m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type");
        updateMode(modeVal);
    }

    bool WifiViewModel::getIsEnabled() const { return m_isEnabled; }

    void WifiViewModel::setIsEnabled(bool enabled) {
        if (enabled != m_isEnabled) {
            m_config->updateSettingByName<bool>("Wireless", "Enabled", enabled);
            m_config->save();
            emit isEnabledChanged();
        }
    }

    QString WifiViewModel::getSelectedInterface() const { return m_selectedInterface; }

    common::Enum::WirelessType::Value WifiViewModel::getMode() const { return m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type"); }

    bool WifiViewModel::getIsHotspot() const { return m_mode == common::Enum::WirelessType::WIRELESS_HOTSPOT; }

    QString WifiViewModel::getHotspotSsid() const {
        return m_hotspotSsid;
    }

    QString WifiViewModel::getHotspotPassword() const {
        return m_hotspotPassword;
    }

    QString WifiViewModel::getClientSsid() const {
        return m_clientSsid;
    }

    QString WifiViewModel::getClientPassword() const {
        return m_clientPassword;
    }

    QString WifiViewModel::getCurrentSsid() const {
        return m_currentSsid;
    }

    int WifiViewModel::getSignalStrength() const {
        return m_signalStrength;
    }

    bool WifiViewModel::getConnected() const {
        return m_connected;
    }

    QVariantList WifiViewModel::getAccessPoints() const {
        return m_accessPoints;
    }


    void WifiViewModel::setSelectedInterface(const QString& iface) {
        if (iface != m_selectedInterface) {
            m_selectedInterface = iface;
            m_config->updateSettingByName<QString>("Wireless", "Interface", iface);
            m_config->save();
            m_wifiController->setInterface(iface);
            emit selectedInterfaceChanged();
        }
    }

    void WifiViewModel::setMode(common::Enum::WirelessType::Value mode) {
        if (mode != this->m_mode) {
            m_config->updateSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", mode);
            m_config->save();
            m_wifiController->setMode(mode);
            emit modeChanged();
        }
    }

    void WifiViewModel::setHotspotSsid(const QString &ssid) {
        if (ssid != m_hotspotSsid) {
            m_hotspotSsid = ssid;
            m_config->updateSettingByName<QString>("Wireless", "HotspotSSID", ssid);
            m_config->save();
            m_wifiController->setHotspotCredentials(m_hotspotSsid, m_hotspotPassword);
            emit hotspotSsidChanged();
        }
    }

    void WifiViewModel::setHotspotPassword(const QString &pass) {
        if (pass != m_hotspotPassword) {
            m_hotspotPassword = pass;
            m_config->updateSettingByName<QString>("Wireless", "HotspotPassword", pass);
            m_config->save();
            m_wifiController->setHotspotCredentials(m_hotspotSsid, m_hotspotPassword);
            emit hotspotPasswordChanged();
        }
    }

    void WifiViewModel::setClientSsid(const QString &ssid) {
        if (ssid != m_clientSsid) {
            m_clientSsid = ssid;
            m_config->updateSettingByName<QString>("Wireless", "ClientSSID", ssid);
            m_config->save();
            m_wifiController->setWirelessCredentials(m_clientSsid, m_clientPassword);
            emit clientSsidChanged();
        }
    }

    void WifiViewModel::setClientPassword(const QString &pass) {
        if (pass != m_clientPassword) {
            m_clientPassword = pass;
            m_config->updateSettingByName<QString>("Wireless", "ClientPassword", pass);
            m_config->save();
            m_wifiController->setWirelessCredentials(m_clientSsid, m_clientPassword);
            emit clientPasswordChanged();
        }
    }

    void WifiViewModel::doWirelessNetworkScan() {
        m_wifiController->scan();
    }

    // Called by WifiMonitor
    void WifiViewModel::updateCurrentSsid(const QString& ssid) {
        if (ssid != m_currentSsid) { m_currentSsid = ssid; emit currentSsidChanged(); }
    }
    void WifiViewModel::updateSignalStrength(int s) {
        if (s != m_signalStrength) { m_signalStrength = s; emit signalStrengthChanged(); }
    }
    void WifiViewModel::updateConnected(bool c) {
        if (c != m_connected) { m_connected = c; emit connectedChanged(); }
    }
    void WifiViewModel::updateMode(common::Enum::WirelessType::Value m) {
        if (m != m_mode) {
            m_config->updateSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", m);
            m_config->save();
            emit modeChanged();
        }
    }

    void WifiViewModel::updateAccessPoints(const QVariantList& aps) {
        m_accessPoints = aps;
        emit accessPointsChanged();
    }
}
