#pragma once
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QJsonObject>
#include <qloggingcategory.h>

Q_DECLARE_LOGGING_CATEGORY(hardwareDetect)

namespace f1x::openauto::autoapp::Hardware {

struct HardwareInfo {
    // Display
    QString primaryDisplay{"hdmi"};
    bool dsiPresent{false};
    bool hdmiPresent{false};

    // Audio
    bool iqaudioDac{false};
    bool onboardAudio{false};
    QStringList usbAudio;
    QStringList usbMicrophone;

    // Connectivity
    bool wifiOnboard{false};
    bool bluetoothOnboard{false};
    bool wifiUsb{false};
    bool bluetoothUsb{false};

    // Camera
    bool piCamera{false};
    QStringList usbCameras;

    // GPS
    bool gpsPresent{false};
    QString gpsDevice;

    // HATs
    bool hatIqaudioDac{false};
    bool hatCanBus{false};
    bool hatRtc{false};
    bool hatGps{false};
};

class HardwareDetector {
public:
    virtual ~HardwareDetector() = default;

    virtual HardwareInfo detect() = 0;

    static HardwareDetector* create();
    static QString journeyOsRuntimePath(const QString& filename);
    static QJsonObject toJson(const HardwareInfo& info);
    static bool writeOutputFiles(const HardwareInfo& info);
};

} // namespace f1x::openauto::autoapp::Hardware
