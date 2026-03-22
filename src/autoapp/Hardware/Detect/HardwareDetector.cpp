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
#ifdef JOURNEYOS_RPI_DETECTION
#include <unistd.h>
#include <sys/reboot.h>
#endif

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
    hats["iqaudio_dac"]    = info.hatIqaudioDac;
    hats["can_bus"]        = info.hatCanBus;
    hats["rtc"]            = info.hatRtc;
    hats["gps_hat"]        = info.hatGps;
    hats["pimoroni_audio"] = info.hatPimoroniAudio;

    QJsonObject root;
    root["display"]      = display;
    root["audio"]        = audio;
    root["connectivity"] = connectivity;
    root["camera"]       = camera;
    root["gps"]          = gps;
    root["hats"]         = hats;
    return root;
}

static void writeDisplayConf(const HardwareInfo& info)
{
    QString content;
    content += QString("DISPLAY_TYPE=%1\n").arg(info.primaryDisplay);
    content += QString("DSI_PRESENT=%1\n").arg(info.dsiPresent ? "1" : "0");
    content += QString("HDMI_PRESENT=%1\n").arg(info.hdmiPresent ? "1" : "0");

    const QString path = HardwareDetector::journeyOsRuntimePath("display.conf");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(hardwareDetect) << "Cannot write" << path << ":" << file.errorString();
        return;
    }
    file.write(content.toUtf8());
    file.close();
    qCInfo(hardwareDetect) << "Written:" << path;
}

static void writeAudioConf(const HardwareInfo& info)
{
    QString hat = "none";
    QString overlay;
    if (info.hatIqaudioDac || info.iqaudioDac) {
        hat = "iqaudio-dacplus";
        overlay = "iqaudio-dacplus";
    } else if (info.hatPimoroniAudio) {
        hat = "pimoroni-audio";
        // Overlay name depends on specific Pimoroni product — configure manually
    }

    QString content;
    content += QString("AUDIO_HAT=%1\n").arg(hat);
    content += QString("AUDIO_HAT_OVERLAY=%1\n").arg(overlay);
    content += QString("ONBOARD_AUDIO=%1\n").arg(info.onboardAudio ? "1" : "0");
    content += QString("USB_AUDIO=%1\n").arg(info.usbAudio.isEmpty() ? "0" : "1");

    const QString path = HardwareDetector::journeyOsRuntimePath("audio.conf");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(hardwareDetect) << "Cannot write" << path << ":" << file.errorString();
        return;
    }
    file.write(content.toUtf8());
    file.close();
    qCInfo(hardwareDetect) << "Written:" << path;
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
    writeDisplayConf(info);
    writeAudioConf(info);
    writeKmsJson(info);
    writeQtEnv(info);
#endif

    return true;
}

bool HardwareDetector::manageBootOverlays(const HardwareInfo& info)
{
#ifndef JOURNEYOS_RPI_DETECTION
    Q_UNUSED(info)
    return false;
#else
    // Build the expected overlay list from detected hardware, in deterministic order.
    // Each entry is a full dtoverlay= line as it would appear in /boot/hardware.txt.
    QStringList overlays;
    if (info.dsiPresent)
        overlays << QStringLiteral("dtoverlay=vc4-kms-dsi-7inch");
    if (info.hatIqaudioDac || info.iqaudioDac)
        overlays << QStringLiteral("dtoverlay=iqaudio-dacplus");
    if (info.hatCanBus)
        overlays << QStringLiteral("dtoverlay=mcp2515-can0");
    if (info.hatRtc)
        overlays << QStringLiteral("dtoverlay=i2c-rtc,ds3231");
    // hatPimoroniAudio: overlay name is product-specific — skip auto-generation
    overlays.sort();

    // Read and parse any existing /boot/hardware.txt
    QStringList existingOverlays;
    QFile hwFile("/boot/hardware.txt");
    if (hwFile.open(QIODevice::ReadOnly)) {
        const QString existing = QString::fromUtf8(hwFile.readAll());
        hwFile.close();
        for (const QString& line : existing.split('\n')) {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty() && !trimmed.startsWith('#'))
                existingOverlays << trimmed;
        }
        existingOverlays.sort();
        qCInfo(hardwareDetect) << "Existing /boot/hardware.txt overlays:" << existingOverlays;
    } else {
        qCInfo(hardwareDetect) << "/boot/hardware.txt absent (first boot)";
    }

    if (overlays == existingOverlays) {
        qCInfo(hardwareDetect) << "Boot overlays unchanged — no reboot needed";
        return false;
    }

    qCInfo(hardwareDetect) << "Boot overlays changed from" << existingOverlays << "to" << overlays;

    // Write new /boot/hardware.txt; fsync required for FAT32 reliability
    {
        QFile outFile("/boot/hardware.txt");
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qCCritical(hardwareDetect) << "Cannot write /boot/hardware.txt:" << outFile.errorString();
            return false;
        }
        QString content;
        content += QStringLiteral("# JourneyOS hardware overlays — auto-generated by journeyos-hardware-detect\n");
        content += QStringLiteral("# Do not edit manually; this file is overwritten on every boot.\n");
        for (const QString& line : overlays)
            content += line + '\n';
        outFile.write(content.toUtf8());
        outFile.flush();
        ::fsync(outFile.handle());
        outFile.close();
        qCInfo(hardwareDetect) << "Written /boot/hardware.txt:" << overlays.size() << "overlay(s)";
    }

    // Write reboot-required flag so other units can check it
    {
        const QString flagPath = journeyOsRuntimePath("reboot-required");
        QFile flagFile(flagPath);
        if (flagFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            flagFile.write("1\n");
            flagFile.close();
            qCInfo(hardwareDetect) << "Written:" << flagPath;
        }
    }

    // Flush all kernel buffers before the raw reboot
    qCInfo(hardwareDetect) << "Triggering reboot for overlay changes to take effect";
    ::sync();
    ::reboot(RB_AUTOBOOT); // does not return on success; requires root

    // Only reached if reboot(2) fails (should not happen as we run as root)
    qCCritical(hardwareDetect) << "reboot(2) failed — reboot manually";
    return true;
#endif
}

} // namespace f1x::openauto::autoapp::Hardware
