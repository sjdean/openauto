#include "f1x/openauto/autoapp/Hardware/Detect/QtHardwareDetector.hpp"

#include <QCoreApplication>
#include <QNetworkInterface>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QCameraDevice>
#include <QBluetoothLocalDevice>

#ifdef QT_GUI_LIB
#include <QGuiApplication>
#include <QScreen>
#endif

namespace f1x::openauto::autoapp::Hardware {

HardwareInfo QtHardwareDetector::detect()
{
    HardwareInfo info;
    try { detectDisplay(info); }      catch (...) { qCWarning(hardwareDetect) << "Display detection failed"; }
    try { detectAudio(info); }        catch (...) { qCWarning(hardwareDetect) << "Audio detection failed"; }
    try { detectConnectivity(info); } catch (...) { qCWarning(hardwareDetect) << "Connectivity detection failed"; }
    try { detectCamera(info); }       catch (...) { qCWarning(hardwareDetect) << "Camera detection failed"; }
    try { detectGps(info); }          catch (...) { qCWarning(hardwareDetect) << "GPS detection failed"; }
    try { detectHats(info); }         catch (...) { qCWarning(hardwareDetect) << "HAT detection failed"; }
    return info;
}

void QtHardwareDetector::detectDisplay(HardwareInfo& info)
{
#ifdef QT_GUI_LIB
    auto* guiApp = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (guiApp) {
        for (QScreen* screen : guiApp->screens()) {
            const QString name = screen->name().toUpper();
            qCInfo(hardwareDetect) << "Screen found:" << screen->name();
            if (name.contains("DSI"))  info.dsiPresent = true;
            if (name.contains("HDMI")) info.hdmiPresent = true;
        }
    } else {
        qCInfo(hardwareDetect) << "Display detection: no QGuiApplication — defaulting to hdmi";
    }
#else
    qCInfo(hardwareDetect) << "Display detection: Qt Gui not available — defaulting to hdmi";
#endif

    if (info.dsiPresent)       info.primaryDisplay = "dsi";
    else if (info.hdmiPresent) info.primaryDisplay = "hdmi";
    else                       info.primaryDisplay = "hdmi"; // safe desktop default
}

void QtHardwareDetector::detectAudio(HardwareInfo& info)
{
    static const QStringList onboardPatterns{"built-in", "default", "bcm", "vc4",
                                             "internal", "speaker", "headphones"};

    for (const QAudioDevice& dev : QMediaDevices::audioOutputs()) {
        const QString desc = dev.description().toLower();
        bool isOnboard = false;
        for (const QString& pat : onboardPatterns) {
            if (desc.contains(pat)) { isOnboard = true; break; }
        }
        if (isOnboard) {
            info.onboardAudio = true;
            qCInfo(hardwareDetect) << "Onboard audio output:" << dev.description();
        } else {
            info.usbAudio.append(dev.description());
            qCInfo(hardwareDetect) << "USB/external audio output:" << dev.description();
        }
    }

    for (const QAudioDevice& dev : QMediaDevices::audioInputs()) {
        const QString desc = dev.description().toLower();
        bool isOnboard = false;
        for (const QString& pat : onboardPatterns) {
            if (desc.contains(pat)) { isOnboard = true; break; }
        }
        if (!isOnboard) {
            info.usbMicrophone.append(dev.description());
            qCInfo(hardwareDetect) << "USB/external microphone:" << dev.description();
        }
    }

    if (!info.onboardAudio) qCInfo(hardwareDetect) << "Onboard audio: not detected";
}

void QtHardwareDetector::detectConnectivity(HardwareInfo& info)
{
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        const QString name = iface.name();
        if (name.startsWith("wlan") || name.startsWith("wlp")) {
            info.wifiOnboard = true;
            qCInfo(hardwareDetect) << "WiFi interface:" << name;
        }
    }
    if (!info.wifiOnboard) qCInfo(hardwareDetect) << "WiFi: not detected";

    if (!QBluetoothLocalDevice::allDevices().isEmpty()) {
        info.bluetoothOnboard = true;
        qCInfo(hardwareDetect) << "Bluetooth adapter: detected";
    } else {
        qCInfo(hardwareDetect) << "Bluetooth adapter: not detected";
    }
}

void QtHardwareDetector::detectCamera(HardwareInfo& info)
{
    for (const QCameraDevice& cam : QMediaDevices::videoInputs()) {
        const QString desc = cam.description().toLower();
        if (desc.contains("mmal") || desc.contains("unicam")) {
            info.piCamera = true;
            qCInfo(hardwareDetect) << "Pi Camera: detected (" << cam.description() << ")";
        } else {
            info.usbCameras.append(cam.description());
            qCInfo(hardwareDetect) << "Camera:" << cam.description();
        }
    }
}

void QtHardwareDetector::detectGps(HardwareInfo& /*info*/)
{
    qCInfo(hardwareDetect) << "GPS detection: not available via Qt — requires platform-specific probing";
}

void QtHardwareDetector::detectHats(HardwareInfo& /*info*/)
{
    qCInfo(hardwareDetect) << "HAT detection: not available via Qt — requires RPi I2C/device-tree probing";
}

} // namespace f1x::openauto::autoapp::Hardware
