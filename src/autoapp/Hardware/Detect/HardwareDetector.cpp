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
#endif

    return true;
}

bool HardwareDetector::manageBootOverlays(const HardwareInfo& info)
{
#ifndef JOURNEYOS_RPI_DETECTION
    Q_UNUSED(info)
    return false;
#else
    // ── /boot availability ────────────────────────────────────────────────────
    // boot-partition.service mounts /boot before this service runs.
    // Bail out rather than write to rootfs if it is missing.
    // Check that /boot is actually mounted, not just that the empty mountpoint exists.
    // config.txt is always present on the boot vfat partition.
    if (!QFile::exists(QStringLiteral("/boot/config.txt"))) {
        qCWarning(hardwareDetect) << "/boot not mounted (config.txt absent) — cannot manage hardware.txt";
        return false;
    }

    const QString bootHwTxt     = QStringLiteral("/boot/hardware.txt");
    const QString stateFilePath = QStringLiteral("/data/journeyos/display-probe-state");

    // ── Display probe state machine ───────────────────────────────────────────
    // DSI overlays are loaded by the bootloader before the kernel starts.
    // DRM sysfs can only confirm DSI is active on the boot *after* the overlay
    // was written.  The state machine tries TD2, then the original Touch Screen,
    // then falls back to HDMI.  Once a display is confirmed it re-writes the same
    // overlay on every boot (idempotent, handles OTA-reset boot volumes).
    //
    // To force a re-probe after changing the display:
    //   rm /data/journeyos/display-probe-state

    QString state;
    {
        QFile sf(stateFilePath);
        if (sf.open(QIODevice::ReadOnly))
            state = QString::fromUtf8(sf.readAll()).trimmed();
    }
    if (state.isEmpty())
        state = QStringLiteral("probe-td2");

    qCInfo(hardwareDetect) << "Display probe state:" << state;

    auto writeState = [&](const QString& newState) {
        QDir().mkpath(QFileInfo(stateFilePath).dir().absolutePath());
        QFile sf(stateFilePath);
        if (sf.open(QIODevice::WriteOnly | QIODevice::Truncate))
            sf.write((newState + '\n').toUtf8());
        qCInfo(hardwareDetect) << "Display probe state ->" << newState;
    };

    // displayOverlay is the dtoverlay= line for the display (empty = HDMI/none).
    // needsReboot is set when we are advancing through probe states — the new
    // overlay takes effect on the next boot, so we reboot immediately.
    QString displayOverlay;
    bool    needsReboot = false;

    // Overlay probe order — both display types tried on DSI1 then DSI0:
    //   1. Touch Display 2 on DSI1 — vc4-kms-dsi-ili9881-7inch (720×1280, rotation=270)
    //   2. Touch Display 2 on DSI0 — same overlay + ,dsi0
    //   3. Original Touch Display on DSI1 — vc4-kms-dsi-7inch
    //   4. Original Touch Display on DSI0 — same overlay + ,dsi0
    //   5. HDMI fallback (no overlay)
    //
    // Worst-case convergence: 5 reboots (display on DSI0, only tried after DSI1 fails).
    // Common case: 1-2 reboots (display on DSI1).
    //
    // To force re-probe after changing display or connector:
    //   rm /data/journeyos/display-probe-state

    static const QString TD2_DSI1 = QStringLiteral("dtoverlay=vc4-kms-dsi-ili9881-7inch");
    static const QString TD2_DSI0 = QStringLiteral("dtoverlay=vc4-kms-dsi-ili9881-7inch,dsi0");
    static const QString TS1_DSI1 = QStringLiteral("dtoverlay=vc4-kms-dsi-7inch");
    static const QString TS1_DSI0 = QStringLiteral("dtoverlay=vc4-kms-dsi-7inch,dsi0");

    if (state == QStringLiteral("probe-td2")) {
        displayOverlay = TD2_DSI1;
        writeState(QStringLiteral("check-td2"));
        needsReboot = true;
        qCInfo(hardwareDetect) << "Probing: Touch Display 2 on DSI1";
    }
    else if (state == QStringLiteral("check-td2")) {
        if (info.dsiPresent) {
            displayOverlay = TD2_DSI1;
            writeState(QStringLiteral("done-td2"));
            qCInfo(hardwareDetect) << "Touch Display 2 on DSI1 confirmed";
        } else {
            displayOverlay = TD2_DSI0;
            writeState(QStringLiteral("check-td2-dsi0"));
            needsReboot = true;
            qCInfo(hardwareDetect) << "TD2/DSI1 not found — probing Touch Display 2 on DSI0";
        }
    }
    else if (state == QStringLiteral("check-td2-dsi0")) {
        if (info.dsiPresent) {
            displayOverlay = TD2_DSI0;
            writeState(QStringLiteral("done-td2-dsi0"));
            qCInfo(hardwareDetect) << "Touch Display 2 on DSI0 confirmed";
        } else {
            displayOverlay = TS1_DSI1;
            writeState(QStringLiteral("check-ts1"));
            needsReboot = true;
            qCInfo(hardwareDetect) << "TD2 not found — probing original Touch Display on DSI1";
        }
    }
    else if (state == QStringLiteral("check-ts1")) {
        if (info.dsiPresent) {
            displayOverlay = TS1_DSI1;
            writeState(QStringLiteral("done-ts1"));
            qCInfo(hardwareDetect) << "Original Touch Display on DSI1 confirmed";
        } else {
            displayOverlay = TS1_DSI0;
            writeState(QStringLiteral("check-ts1-dsi0"));
            needsReboot = true;
            qCInfo(hardwareDetect) << "TS1/DSI1 not found — probing original Touch Display on DSI0";
        }
    }
    else if (state == QStringLiteral("check-ts1-dsi0")) {
        if (info.dsiPresent) {
            displayOverlay = TS1_DSI0;
            writeState(QStringLiteral("done-ts1-dsi0"));
            qCInfo(hardwareDetect) << "Original Touch Display on DSI0 confirmed";
        } else {
            writeState(QStringLiteral("done-hdmi"));
            needsReboot = true;
            qCInfo(hardwareDetect) << "No DSI display found on any connector — falling back to HDMI";
        }
    }
    else if (state == QStringLiteral("done-td2")) {
        displayOverlay = TD2_DSI1;
    }
    else if (state == QStringLiteral("done-td2-dsi0")) {
        displayOverlay = TD2_DSI0;
    }
    else if (state == QStringLiteral("done-ts1")) {
        displayOverlay = TS1_DSI1;
    }
    else if (state == QStringLiteral("done-ts1-dsi0")) {
        displayOverlay = TS1_DSI0;
    }
    else if (state == QStringLiteral("done-hdmi")) {
        // HDMI — no display overlay
    }
    else {
        qCWarning(hardwareDetect) << "Unknown display probe state:" << state << "— resetting";
        displayOverlay = QStringLiteral("dtoverlay=vc4-kms-dsi-ili9881-7inch");
        writeState(QStringLiteral("check-td2"));
        needsReboot = true;
    }

    // ── Build full overlay list (display + HATs) ──────────────────────────────
    // HATs are detected via I2C/EEPROM every boot — no state machine needed.
    QStringList overlays;
    if (!displayOverlay.isEmpty())
        overlays << displayOverlay;
    if (info.hatIqaudioDac || info.iqaudioDac)
        overlays << QStringLiteral("dtoverlay=iqaudio-dacplus");
    if (info.hatCanBus)
        overlays << QStringLiteral("dtoverlay=mcp2515-can0");
    if (info.hatRtc)
        overlays << QStringLiteral("dtoverlay=i2c-rtc,ds3231");
    // hatPimoroniAudio: overlay name is product-specific — configure manually

    // ── Write /boot/hardware.txt (always — idempotent, handles OTA resets) ───
    {
        QFile f(bootHwTxt);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qCCritical(hardwareDetect) << "Cannot write" << bootHwTxt << ":" << f.errorString();
            return false;
        }
        QString content;
        content += QStringLiteral("# JourneyOS hardware overlays\n");
        content += QStringLiteral("# Populated by journeyos-hardware-detect on first boot and after OTA updates.\n");
        content += QStringLiteral("# Do not edit manually — changes will be overwritten by the hardware detector.\n");
        for (const QString& line : overlays)
            content += line + '\n';
        f.write(content.toUtf8());
        f.flush();
        ::fsync(f.handle());
        f.close();
        qCInfo(hardwareDetect) << "Written" << bootHwTxt
                               << (overlays.isEmpty() ? "(HDMI/no overlay)" : overlays.join(", "));
    }

    if (!needsReboot)
        return false;

    // ── Reboot to activate new overlay ───────────────────────────────────────
    qCInfo(hardwareDetect) << "Rebooting to activate overlay changes";
    ::sync();
    ::reboot(RB_AUTOBOOT); // does not return on success (requires root)

    qCCritical(hardwareDetect) << "reboot(2) failed — reboot manually";
    return true;
#endif
}

} // namespace f1x::openauto::autoapp::Hardware
