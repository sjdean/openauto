#pragma once

#include <QObject>
#include <QString>

#include "f1x/openauto/Common/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::UI::Controller {

    class IWiFiController : public QObject {
        Q_OBJECT

    public:
        explicit IWiFiController(QObject *parent = nullptr) : QObject(parent) {}
        ~IWiFiController() override = default;

        virtual void setInterface(const QString &ifaceName) = 0;
        virtual void setMode(common::Enum::WirelessType::Value mode) = 0;
        virtual void setHotspotCredentials(const QString &ssid, const QString &password) = 0;
        virtual void setWirelessCredentials(const QString &ssid, const QString &password) = 0;
        virtual void scan() = 0;
        virtual void disconnect() = 0;
        virtual void connectToNetwork(const QString &ssid, const QString &password) = 0;

    signals:
        void errorOccurred(const QString &message);
        void statusChanged(const QString &message);
    };

} // namespace
