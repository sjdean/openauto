#pragma once
#include "f1x/openauto/autoapp/Hardware/Detect/HardwareDetector.hpp"

namespace f1x::openauto::autoapp::Hardware {

class QtHardwareDetector : public HardwareDetector {
public:
    HardwareInfo detect() override;

protected:
    virtual void detectDisplay(HardwareInfo& info);
    virtual void detectAudio(HardwareInfo& info);
    virtual void detectConnectivity(HardwareInfo& info);
    virtual void detectCamera(HardwareInfo& info);
    virtual void detectGps(HardwareInfo& info);
    virtual void detectHats(HardwareInfo& info);
};

} // namespace f1x::openauto::autoapp::Hardware
