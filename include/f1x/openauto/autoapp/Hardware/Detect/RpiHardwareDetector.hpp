#pragma once
#include <QtGlobal>

// Ensure this is visible so HardwareInfo is defined
#include "f1x/openauto/autoapp/Hardware/Detect/QtHardwareDetector.hpp"

namespace f1x::openauto::autoapp::Hardware {

    class RpiHardwareDetector : public QtHardwareDetector {
    public:
        HardwareInfo detect() override;

    protected:
        void detectDisplay(HardwareInfo& info) override;
        void detectAudio(HardwareInfo& info) override;
        void detectConnectivity(HardwareInfo& info) override;
        void detectCamera(HardwareInfo& info) override;
        void detectGps(HardwareInfo& info) override;
        void detectHats(HardwareInfo& info) override;

    private:
        bool probeI2cAddress(int bus, quint8 address);
        void detectHatEeprom(HardwareInfo& info);
    };

} // namespace f1x::openauto::autoapp::Hardware