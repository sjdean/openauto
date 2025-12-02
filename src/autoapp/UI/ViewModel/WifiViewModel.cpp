// WifiViewModel.cpp
#include "f1x/openauto/autoapp/UI/ViewModel/WifiViewModel.hpp"
#include <QDebug>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {


    WifiViewModel::WifiViewModel(configuration::IConfiguration::Pointer config, QObject* parent)
        : QObject(parent), m_config(std::move(config))
    {
        // Load from config
        m_selectedInterface = m_config->getSettingByName<QString>("Wireless", "Interface");
        m_hotspotSsid       = m_config->getSettingByName<QString>("Wireless", "HotspotSSID");
        m_hotspotPassword   = m_config->getSettingByName<QString>("Wireless", "HotspotPassword");
        m_clientSsid        = m_config->getSettingByName<QString>("Wireless", "ClientSSID");
        m_clientPassword    = m_config->getSettingByName<QString>("Wireless", "ClientPassword");
        auto modeVal        = m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type");
        updateMode(modeVal); // sets internal + emits
    }

    QString WifiViewModel::selectedInterface() const { return m_selectedInterface; }
    common::Enum::WirelessType::Value WifiViewModel::mode() const { return m_config->getSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type"); }
    bool WifiViewModel::isHotspot() const { return mode() == common::Enum::WirelessType::WIRELESS_HOTSPOT; }

    QString WifiViewModel::hotspotSsid() const {
        return m_hotspotSsid;
    }


    QString WifiViewModel::hotspotPassword() const {
        return m_hotspotPassword;
    }

    QString WifiViewModel::clientSsid() const {
        return m_clientSsid;
    }

    QString WifiViewModel::clientPassword() const {
        return m_clientPassword;
    }

    QString WifiViewModel::currentSsid() const {
        return m_currentSsid;
    }

    int WifiViewModel::signalStrength() const {
        return m_signalStrength;
    }

    bool WifiViewModel::connected() const {
        return m_connected;
    }

    QVariantList WifiViewModel::accessPoints() const {
        return m_accessPoints;
    }


    void WifiViewModel::setSelectedInterface(const QString& iface) {
        if (iface != m_selectedInterface) {
            m_selectedInterface = iface;
            m_config->updateSettingByName<QString>("Wireless", "Interface", iface);
            m_config->save();
            emit selectedInterfaceChanged();
        }
    }

    void WifiViewModel::setMode(common::Enum::WirelessType::Value mode) {
        if (mode != this->mode()) {
            m_config->updateSettingByName<common::Enum::WirelessType::Value>("Wireless", "Type", mode);
            m_config->save();
            emit modeChanged();
        }
    }

    void WifiViewModel::setHotspotSsid(const QString& ssid) {
        if (ssid != m_hotspotSsid) {
            m_hotspotSsid = ssid;
            m_config->updateSettingByName<QString>("Wireless", "HotspotSSID", ssid);
            m_config->save();
            emit hotspotSsidChanged();
        }
    }

    void WifiViewModel::setHotspotPassword(const QString& pass) {
        if (pass != m_hotspotPassword) {
            m_hotspotPassword = pass;
            m_config->updateSettingByName<QString>("Wireless", "HotspotPassword", pass);
            m_config->save();
            emit hotspotPasswordChanged();
        }
    }

    void WifiViewModel::setClientSsid(const QString& ssid) {
        if (ssid != m_clientSsid) {
            m_clientSsid = ssid;
            m_config->updateSettingByName<QString>("Wireless", "ClientSSID", ssid);
            m_config->save();
            emit clientSsidChanged();
        }
    }

    void WifiViewModel::setClientPassword(const QString& pass) {
        if (pass != m_clientPassword) {
            m_clientPassword = pass;
            m_config->updateSettingByName<QString>("Wireless", "ClientPassword", pass);
            m_config->save();
            emit clientPasswordChanged();
        }
    }

    void WifiViewModel::requestScan() { emit scanRequested(); }
    void WifiViewModel::connectToAp(const QString& ssid) { emit connectRequested(ssid); }

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
        auto current = mode();
        if (m != current) {
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
