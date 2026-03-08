#include <QGuiApplication>
#include <QDir>
#include <QFileInfo>
#include <memory>

#include "f1x/openauto/autoapp/Hardware/Detect/HardwareDetector.hpp"

int main(int argc, char* argv[])
{
#ifdef JOURNEYOS_RPI_DETECTION
    // Running as systemd service before Weston starts — use offscreen platform.
    // RpiHardwareDetector uses sysfs/EDID/I2C, not Qt display APIs.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName("journeyos-hardware-detect");

    using namespace f1x::openauto::autoapp::Hardware;

    // Ensure the runtime output directory exists
    const QString runtimeDir = QFileInfo(HardwareDetector::journeyOsRuntimePath("hardware.json"))
                                   .dir().absolutePath();
    if (!QDir().mkpath(runtimeDir)) {
        qCCritical(hardwareDetect) << "Cannot create output directory:" << runtimeDir;
        return 1;
    }

    std::unique_ptr<HardwareDetector> detector(HardwareDetector::create());

    HardwareInfo info;
    try {
        info = detector->detect();
    } catch (const std::exception& ex) {
        qCWarning(hardwareDetect) << "Detection threw exception:" << ex.what() << "— using defaults";
    } catch (...) {
        qCWarning(hardwareDetect) << "Detection threw unknown exception — using defaults";
    }

    return HardwareDetector::writeOutputFiles(info) ? 0 : 1;
}
