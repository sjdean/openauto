#include "f1x/openauto/autoapp/Hardware/Detect/HardwareDetector.hpp"
#include "f1x/openauto/autoapp/Hardware/Detect/QtHardwareDetector.hpp"
#ifdef JOURNEYOS_RPI_DETECTION
#include "f1x/openauto/autoapp/Hardware/Detect/RpiHardwareDetector.hpp"
#endif

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(hardwareDetect, "journeyos.hardware")

namespace f1x::openauto::autoapp::Hardware {

HardwareDetector* HardwareDetector::create()
{
#ifdef JOURNEYOS_RPI_DETECTION
    QFile model("/proc/device-tree/model");
    if (model.open(QIODevice::ReadOnly)) {
        if (model.readAll().contains("Raspberry Pi"))
            return new RpiHardwareDetector();
    }
#endif
    return new QtHardwareDetector();
}

QString HardwareDetector::journeyOsRuntimePath(const QString& filename)
{
#ifdef Q_OS_LINUX
    return "/run/journeyos/" + filename;
#else
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation)
           + "/journeyos/" + filename;
#endif
}

QJsonObject HardwareDetector::toJson(const HardwareInfo& info)
{
    QJsonObject display;
    display["primary"]      = info.primaryDisplay;
    display["dsi_present"]  = info.dsiPresent;
    display["hdmi_present"] = info.hdmiPresent;

    QJsonArray usbAudioArr;
    for (const QString& s : info.usbAudio)       usbAudioArr.append(s);
    QJsonArray usbMicArr;
    for (const QString& s : info.usbMicrophone)  usbMicArr.append(s);

    QJsonObject audio;
    audio["iqaudio_dac"]    = info.iqaudioDac;
    audio["onboard"]        = info.onboardAudio;
    audio["usb_audio"]      = usbAudioArr;
    audio["usb_microphone"] = usbMicArr;

    QJsonObject connectivity;
    connectivity["wifi_onboard"]      = info.wifiOnboard;
    connectivity["bluetooth_onboard"] = info.bluetoothOnboard;
    connectivity["wifi_usb"]          = info.wifiUsb;
    connectivity["bluetooth_usb"]     = info.bluetoothUsb;

    QJsonArray usbCamArr;
    for (const QString& s : info.usbCameras) usbCamArr.append(s);

    QJsonObject camera;
    camera["pi_camera"]  = info.piCamera;
    camera["usb_cameras"] = usbCamArr;

    QJsonObject gps;
    gps["present"] = info.gpsPresent;
    gps["device"]  = info.gpsDevice;

    QJsonObject hats;
    hats["iqaudio_dac"] = info.hatIqaudioDac;
    hats["can_bus"]     = info.hatCanBus;
    hats["rtc"]         = info.hatRtc;
    hats["gps_hat"]     = info.hatGps;

    QJsonObject root;
    root["display"]      = display;
    root["audio"]        = audio;
    root["connectivity"] = connectivity;
    root["camera"]       = camera;
    root["gps"]          = gps;
    root["hats"]         = hats;
    return root;
}

static void writeKmsJson(const HardwareInfo& info)
{
    QJsonArray outputs;
    if (info.primaryDisplay == "dsi") {
        QJsonObject dsi;  dsi["name"] = "DSI-1";   dsi["primary"] = true;   outputs.append(dsi);
        QJsonObject hdmi; hdmi["name"] = "HDMI-1";  hdmi["enabled"] = false; outputs.append(hdmi);
    } else {
        QJsonObject hdmi; hdmi["name"] = "HDMI-1";  hdmi["primary"] = true;  outputs.append(hdmi);
        QJsonObject dsi;  dsi["name"] = "DSI-1";   dsi["enabled"] = false;  outputs.append(dsi);
    }
    QJsonObject kms;
    kms["device"]  = "/dev/dri/card0";
    kms["outputs"] = outputs;

    const QString path = HardwareDetector::journeyOsRuntimePath("kms.json");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(hardwareDetect) << "Cannot write" << path << ":" << file.errorString();
        return;
    }
    file.write(QJsonDocument(kms).toJson(QJsonDocument::Indented));
    file.close();
    qCInfo(hardwareDetect) << "Written:" << path;
}

static void writeQtEnv(const HardwareInfo& info)
{
    QString content;
    if (info.primaryDisplay == "dsi") {
        content = "QT_QPA_PLATFORM=eglfs\n"
                  "QT_QPA_EGLFS_KMS_CONFIG=/run/journeyos/kms.json\n"
                  "QT_QPA_EGLFS_KMS_ATOMIC=1\n";
    } else {
        // "hdmi" or "none"
        content = "QT_QPA_PLATFORM=wayland\n";
    }

    const QString path = HardwareDetector::journeyOsRuntimePath("qt.env");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(hardwareDetect) << "Cannot write" << path << ":" << file.errorString();
        return;
    }
    file.write(content.toUtf8());
    file.close();
    qCInfo(hardwareDetect) << "Written:" << path;
}

bool HardwareDetector::writeOutputFiles(const HardwareInfo& info)
{
    const QString hwPath = journeyOsRuntimePath("hardware.json");
    QFile hwFile(hwPath);
    if (!hwFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCCritical(hardwareDetect) << "Cannot write" << hwPath << ":" << hwFile.errorString();
        return false;
    }
    hwFile.write(QJsonDocument(toJson(info)).toJson(QJsonDocument::Indented));
    hwFile.close();
    qCInfo(hardwareDetect) << "Written:" << hwPath;

#ifdef JOURNEYOS_RPI_DETECTION
    writeKmsJson(info);
    writeQtEnv(info);
#endif

    return true;
}

} // namespace f1x::openauto::autoapp::Hardware
