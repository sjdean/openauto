#pragma once

#include "f1x/openauto/autoapp/UI/Controller/IWiFiController.hpp"

namespace f1x::openauto::autoapp::UI::Controller {

    class NullWiFiController : public IWiFiController {
        Q_OBJECT

    public:
        explicit NullWiFiController(QObject *parent = nullptr) : IWiFiController(parent) {}

        void setInterface(const QString &) override {}
        void setMode(common::Enum::WirelessType::Value) override {}
        void setHotspotCredentials(const QString &, const QString &) override {}
        void setWirelessCredentials(const QString &, const QString &) override {}
        void scan() override {}
        void disconnect() override {}
        void connectToNetwork(const QString &, const QString &) override {}
    };

} // namespace
