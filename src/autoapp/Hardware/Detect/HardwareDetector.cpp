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
    const QString failCountPath = QStringLiteral("/data/journeyos/display-probe-fail-count");

    // ── Display probe state machine ───────────────────────────────────────────
    // DSI overlays are loaded by the bootloader before the kernel starts.
    // DRM sysfs can only confirm DSI is active on the boot *after* the overlay
    // was written.  The state machine tries TD2 on DSI1, then DSI0, then the
    // original Touch Screen on DSI1/DSI0, then falls back to HDMI.
    // Once a display is confirmed it re-writes the same overlay on every boot
    // (idempotent, handles OTA-reset boot volumes).
    //
    // Retry logic: the ILI9881C panel in Touch Display 2 can fail MIPI init on
    // soft reboots, reporting the connector as disconnected even when physically
    // present.  We require kProbeRetries consecutive failures before advancing
    // to the next overlay — this prevents a single bad boot from permanently
    // locking the wrong overlay.
    //
    // To force a full re-probe after changing the display or connector:
    //   rm /data/journeyos/display-probe-state /data/journeyos/display-probe-fail-count

    // 2 total attempts per DSI candidate before advancing.
    // Cold boots (fresh flash) almost always succeed on attempt 1.
    // The second attempt guards against a single soft-reboot transient.
    static constexpr int kProbeRetries = 2;

    QString state;
    {
        QFile sf(stateFilePath);
        if (sf.open(QIODevice::ReadOnly))
            state = QString::fromUtf8(sf.readAll()).trimmed();
    }
    if (state.isEmpty())
        state = QStringLiteral("probe-td2");

    qCInfo(hardwareDetect) << "Display probe state:" << state;

    int failCount = 0;
    {
        QFile cf(failCountPath);
        if (cf.open(QIODevice::ReadOnly))
            failCount = QString::fromUtf8(cf.readAll()).trimmed().toInt();
    }

    auto writeState = [&](const QString& newState) {
        QDir().mkpath(QFileInfo(stateFilePath).dir().absolutePath());
        QFile sf(stateFilePath);
        if (sf.open(QIODevice::WriteOnly | QIODevice::Truncate))
            sf.write((newState + '\n').toUtf8());
        qCInfo(hardwareDetect) << "Display probe state ->" << newState;
    };

    auto writeFailCount = [&](int n) {
        QDir().mkpath(QFileInfo(failCountPath).dir().absolutePath());
        QFile cf(failCountPath);
        if (cf.open(QIODevice::WriteOnly | QIODevice::Truncate))
            cf.write((QString::number(n) + '\n').toUtf8());
        qCInfo(hardwareDetect) << "Display probe fail count ->" << n;
    };

    auto resetFailCount = [&]() {
        QFile::remove(failCountPath);
        failCount = 0;
    };

    // displayOverlay is the dtoverlay= line for the display (empty = HDMI/none).
    // needsReboot is set when a reboot is required (overlay change or retry).
    // Declared here so the handleCheckState lambda below can capture them via [&].
    QString displayOverlay;
    bool    needsReboot = false;

    // Helper: handle the check logic shared by every check-* state.
    // On detection: lock to doneState.
    // On failure with retries remaining: stay on current overlay, reboot.
    // On failure exhausted: advance to nextOverlay / nextState.
    // Returns the overlay string to use for hardware.txt.
    // Sets needsReboot as a side-effect.
    auto handleCheckState = [&](const QString& currentOverlay,
                                const QString& doneState,
                                const QString& nextOverlay,
                                const QString& nextState,
                                const QString& doneMsg,
                                const QString& retryMsg,
                                const QString& advanceMsg) -> QString {
        if (info.dsiPresent) {
            writeState(doneState);
            resetFailCount();
            qCInfo(hardwareDetect) << doneMsg;
            return currentOverlay;
        }
        if (failCount + 1 < kProbeRetries) {
            writeFailCount(failCount + 1);
            needsReboot = true;
            qCInfo(hardwareDetect) << retryMsg
                                   << "(" << (failCount + 1) << "of" << kProbeRetries << ")";
            return currentOverlay; // keep same overlay loaded
        }
        // All retries exhausted — advance
        writeState(nextState);
        resetFailCount();
        needsReboot = true;
        qCInfo(hardwareDetect) << advanceMsg;
        return nextOverlay;
    };

    // Probe order:
    //   0. HDMI (no overlay) — detected instantly on first probe boot, 0 reboots
    //   1. Touch Display 2 on DSI1 — vc4-kms-dsi-ili9881-7inch
    //   2. Touch Display 2 on DSI0 — same overlay + ,dsi0
    //   3. Original Touch Screen on DSI1 — vc4-kms-dsi-7inch
    //   4. Original Touch Screen on DSI0 — same overlay + ,dsi0
    //   5. HDMI fallback (no overlay, if DSI probing exhausted)
    //
    // Worst-case (DSI display but no HDMI connected): 4 candidates × kProbeRetries = 8 reboots.
    // Common cases: HDMI → 0 reboots; TD2 on DSI1 (cold boot) → 1–2 reboots.
    //
    // Note: when re-probing manually, use `poweroff` + cold power-on rather than
    // `reboot` — the ILI9881C panel requires a full power cycle to reset reliably.

    static const QString TD2_DSI1 = QStringLiteral("dtoverlay=vc4-kms-dsi-ili9881-7inch");
    static const QString TD2_DSI0 = QStringLiteral("dtoverlay=vc4-kms-dsi-ili9881-7inch,dsi0");
    static const QString TS1_DSI1 = QStringLiteral("dtoverlay=vc4-kms-dsi-7inch");
    static const QString TS1_DSI0 = QStringLiteral("dtoverlay=vc4-kms-dsi-7inch,dsi0");

    if (state == QStringLiteral("probe-td2")) {
        if (info.hdmiPresent) {
            // HDMI is already active — no overlay needed, no reboot.
            // Skip DSI probing entirely for HDMI-only setups.
            writeState(QStringLiteral("done-hdmi"));
            resetFailCount();
            qCInfo(hardwareDetect) << "HDMI detected on first probe — skipping DSI, no reboot needed";
            // displayOverlay stays empty; needsReboot stays false
        } else {
            displayOverlay = TD2_DSI1;
            writeState(QStringLiteral("check-td2"));
            resetFailCount();
            needsReboot = true;
            qCInfo(hardwareDetect) << "No HDMI detected — probing Touch Display 2 on DSI1";
        }
    }
    else if (state == QStringLiteral("check-td2")) {
        displayOverlay = handleCheckState(
            TD2_DSI1, QStringLiteral("done-td2"),
            TD2_DSI0, QStringLiteral("check-td2-dsi0"),
            QStringLiteral("Touch Display 2 on DSI1 confirmed"),
            QStringLiteral("TD2/DSI1 not detected — retrying"),
            QStringLiteral("TD2/DSI1 failed — probing Touch Display 2 on DSI0"));
    }
    else if (state == QStringLiteral("check-td2-dsi0")) {
        displayOverlay = handleCheckState(
            TD2_DSI0, QStringLiteral("done-td2-dsi0"),
            TS1_DSI1, QStringLiteral("check-ts1"),
            QStringLiteral("Touch Display 2 on DSI0 confirmed"),
            QStringLiteral("TD2/DSI0 not detected — retrying"),
            QStringLiteral("TD2/DSI0 failed — probing original Touch Screen on DSI1"));
    }
    else if (state == QStringLiteral("check-ts1")) {
        displayOverlay = handleCheckState(
            TS1_DSI1, QStringLiteral("done-ts1"),
            TS1_DSI0, QStringLiteral("check-ts1-dsi0"),
            QStringLiteral("Original Touch Screen on DSI1 confirmed"),
            QStringLiteral("TS1/DSI1 not detected — retrying"),
            QStringLiteral("TS1/DSI1 failed — probing original Touch Screen on DSI0"));
    }
    else if (state == QStringLiteral("check-ts1-dsi0")) {
        if (info.dsiPresent) {
            displayOverlay = TS1_DSI0;
            writeState(QStringLiteral("done-ts1-dsi0"));
            resetFailCount();
            qCInfo(hardwareDetect) << "Original Touch Screen on DSI0 confirmed";
        } else if (failCount + 1 < kProbeRetries) {
            displayOverlay = TS1_DSI0;
            writeFailCount(failCount + 1);
            needsReboot = true;
            qCInfo(hardwareDetect) << "TS1/DSI0 not detected — retrying"
                                   << "(" << (failCount + 1) << "of" << kProbeRetries << ")";
        } else {
            writeState(QStringLiteral("done-hdmi"));
            resetFailCount();
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
        displayOverlay = TD2_DSI1;
        writeState(QStringLiteral("check-td2"));
        resetFailCount();
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
