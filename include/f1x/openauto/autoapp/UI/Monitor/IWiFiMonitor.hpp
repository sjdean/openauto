#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include "f1x/openauto/Common/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

    class IWiFiMonitor : public QObject {
        Q_OBJECT

    public:
        explicit IWiFiMonitor(QObject *parent = nullptr) : QObject(parent) {}
        ~IWiFiMonitor() override = default;

        virtual void requestScan() = 0;

    signals:
        void currentSsidChanged(const QString &ssid);
        void signalStrengthChanged(int strength);
        void connectedChanged(bool connected);
        void modeChanged(common::Enum::WirelessType::Value mode);
        void interfaceChanged(const QString &macAddress);
        void interfaceUpChanged(bool up);
        void currentIpChanged(const QString &ip);
        void availableInterfacesChanged(const QVariantList &interfaces);
        void accessPointsChanged(const QVariantList &aps);
    };

} // namespace
