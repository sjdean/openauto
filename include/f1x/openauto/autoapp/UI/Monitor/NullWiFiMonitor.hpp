#pragma once

#include "f1x/openauto/autoapp/UI/Monitor/IWiFiMonitor.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

    class NullWiFiMonitor : public IWiFiMonitor {
        Q_OBJECT

    public:
        explicit NullWiFiMonitor(QObject *parent = nullptr) : IWiFiMonitor(parent) {}

        void requestScan() override {}
    };

} // namespace
