#include "f1x/openauto/autoapp/Hardware/Detect/RpiHardwareDetector.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMediaDevices>
#include <QAudioDevice>

// POSIX / Linux headers for I2C probing
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cerrno>
#include <cstring>

namespace f1x::openauto::autoapp::Hardware {

// ── Public entry point ────────────────────────────────────────────────────────

HardwareInfo RpiHardwareDetector::detect()
{
    HardwareInfo info = QtHardwareDetector::detect(); // dispatches to all overridden methods

    // Post-process: if GPS HAT found and no USB GPS, try serial port
    if (info.hatGps && !info.gpsPresent) {
        if (QFile::exists("/dev/ttyAMA0")) {
            info.gpsPresent = true;
            info.gpsDevice  = "/dev/ttyAMA0";
            qCInfo(hardwareDetect) << "GPS device (serial):" << info.gpsDevice;
        } else if (QFile::exists("/dev/serial0")) {
            info.gpsPresent = true;
            info.gpsDevice  = "/dev/serial0";
            qCInfo(hardwareDetect) << "GPS device (serial):" << info.gpsDevice;
        }
    }
    return info;
}

// ── Display ───────────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectDisplay(HardwareInfo& info)
{
    QDir drmDir("/sys/class/drm/");
    if (!drmDir.exists()) {
        qCWarning(hardwareDetect) << "DRM sysfs not available — display detection skipped";
        info.primaryDisplay = "hdmi";
        return;
    }

    const QStringList entries = drmDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& entry : entries) {
        if (entry.contains("-DSI-", Qt::CaseInsensitive)) {
            QFile statusFile("/sys/class/drm/" + entry + "/status");
            if (statusFile.open(QIODevice::ReadOnly)) {
                if (statusFile.readAll().trimmed() == "connected") {
                    info.dsiPresent = true;
                    qCInfo(hardwareDetect) << "DSI display: connected via" << entry;
                } else {
                    qCInfo(hardwareDetect) << "DSI display: not connected (" << entry << ")";
                }
            }
        } else if (entry.contains("-HDMI-", Qt::CaseInsensitive)) {
            // CRITICAL: use EDID, NOT status — RPi reports HDMI as connected even with nothing plugged in
            QFile edidFile("/sys/class/drm/" + entry + "/edid");
            if (edidFile.open(QIODevice::ReadOnly)) {
                if (!edidFile.readAll().isEmpty()) {
                    info.hdmiPresent = true;
                    qCInfo(hardwareDetect) << "HDMI display: detected via EDID in" << entry;
                } else {
                    qCInfo(hardwareDetect) << "HDMI display: no EDID in" << entry << "(not connected)";
                }
            }
        }
    }

    // Priority: DSI wins if both are detected simultaneously
    if (info.dsiPresent)       info.primaryDisplay = "dsi";
    else if (info.hdmiPresent) info.primaryDisplay = "hdmi";
    else                       info.primaryDisplay = "none";
    qCInfo(hardwareDetect) << "Primary display:" << info.primaryDisplay;
}

// ── Audio ─────────────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectAudio(HardwareInfo& info)
{
    QFile asoundCards("/proc/asound/cards");
    if (!asoundCards.open(QIODevice::ReadOnly)) {
        qCWarning(hardwareDetect) << "Cannot read /proc/asound/cards — audio detection skipped";
        return;
    }

    const QString content = QString::fromUtf8(asoundCards.readAll());

    // Read capture-capable PCM entries once for USB mic detection
    QSet<int> captureCards;
    QFile pcmFile("/proc/asound/pcm");
    if (pcmFile.open(QIODevice::ReadOnly)) {
        const QString pcmContent = QString::fromUtf8(pcmFile.readAll());
        QRegularExpression captureRe(R"(^(\d{2})-\d{2}:.*capture)",
                                     QRegularExpression::MultilineOption |
                                     QRegularExpression::CaseInsensitiveOption);
        auto it = captureRe.globalMatch(pcmContent);
        while (it.hasNext())
            captureCards.insert(it.next().captured(1).toInt());
    }

    // Parse card entries: " N [shortname  ]: driver - long description"
    QRegularExpression cardRe(R"(^\s*(\d+)\s+\[([^\]]+)\]\s*:\s*(\S+)\s+-\s+(.+)$)",
                              QRegularExpression::MultilineOption);
    auto it = cardRe.globalMatch(content);
    while (it.hasNext()) {
        auto m = it.next();
        const int     cardNum   = m.captured(1).toInt();
        const QString shortName = m.captured(2).trimmed();
        const QString driver    = m.captured(3);
        const QString desc      = m.captured(4).trimmed();

        const QString shortLc  = shortName.toLower();
        const QString driverLc = driver.toLower();

        if (shortLc.contains("iqaudio") || driverLc.contains("iqaudio")) {
            info.iqaudioDac = true;
            qCInfo(hardwareDetect) << "IQAudio DAC+: detected in /proc/asound/cards";
            continue;
        }
        if (driverLc.contains("bcm2835") || driverLc.contains("vc4") ||
            shortLc.contains("headphones") || shortLc.contains("hdmi")) {
            info.onboardAudio = true;
            qCInfo(hardwareDetect) << "Onboard audio: detected (" << shortName << ")";
            continue;
        }

        // Check if USB via sysfs subsystem symlink
        const QString subsysLink = QString("/sys/class/sound/card%1/device/subsystem").arg(cardNum);
        QFileInfo fi(subsysLink);
        if (fi.isSymLink() && fi.symLinkTarget().contains("usb", Qt::CaseInsensitive)) {
            info.usbAudio.append(desc);
            qCInfo(hardwareDetect) << "USB audio:" << desc << "(card" << cardNum << ")";
            if (captureCards.contains(cardNum)) {
                info.usbMicrophone.append(desc);
                qCInfo(hardwareDetect) << "USB microphone:" << desc;
            }
        }
    }

    if (!info.iqaudioDac)   qCInfo(hardwareDetect) << "IQAudio DAC+: not detected";
    if (!info.onboardAudio) qCInfo(hardwareDetect) << "Onboard audio: not detected";
}

// ── Connectivity ──────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectConnectivity(HardwareInfo& info)
{
    // WiFi
    QDir netDir("/sys/class/net/");
    if (netDir.exists()) {
        for (const QString& iface : netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (!iface.startsWith("wlan")) continue;
            const QString deviceLink = "/sys/class/net/" + iface + "/device";
            QFileInfo fi(deviceLink);
            const bool isUsb = fi.isSymLink() &&
                               fi.symLinkTarget().contains("usb", Qt::CaseInsensitive);
            if (iface == "wlan0" && !isUsb) {
                info.wifiOnboard = true;
                qCInfo(hardwareDetect) << "WiFi onboard: wlan0 detected";
            }
            if (isUsb) {
                info.wifiUsb = true;
                qCInfo(hardwareDetect) << "WiFi USB adapter:" << iface;
            }
        }
    }
    if (!info.wifiOnboard) qCInfo(hardwareDetect) << "WiFi onboard: not detected";

    // Bluetooth
    QDir btDir("/sys/class/bluetooth/");
    if (btDir.exists()) {
        for (const QString& hci : btDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString deviceLink = "/sys/class/bluetooth/" + hci + "/device";
            QFileInfo fi(deviceLink);
            const bool isUsb = fi.isSymLink() &&
                               fi.symLinkTarget().contains("usb", Qt::CaseInsensitive);
            if (hci == "hci0" && !isUsb) {
                info.bluetoothOnboard = true;
                qCInfo(hardwareDetect) << "Bluetooth onboard: hci0 detected";
            }
            if (isUsb) {
                info.bluetoothUsb = true;
                qCInfo(hardwareDetect) << "Bluetooth USB adapter:" << hci;
            }
        }
    }
    if (!info.bluetoothOnboard) qCInfo(hardwareDetect) << "Bluetooth onboard: not detected";
}

// ── Camera ────────────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectCamera(HardwareInfo& info)
{
    // Pi Camera: /dev/video0 exists AND sysfs name contains "mmal" or "unicam"
    if (QFile::exists("/dev/video0")) {
        QFile nameFile("/sys/class/video4linux/video0/name");
        if (nameFile.open(QIODevice::ReadOnly)) {
            const QString name = QString::fromUtf8(nameFile.readAll()).trimmed().toLower();
            if (name.contains("mmal") || name.contains("unicam")) {
                info.piCamera = true;
                qCInfo(hardwareDetect) << "Pi Camera: detected (" << name << ")";
            }
        }
    }
    if (!info.piCamera) qCInfo(hardwareDetect) << "Pi Camera: not detected";

    // USB cameras: any /dev/video* where sysfs path contains "usb"
    QDir v4lDir("/sys/class/video4linux/");
    if (v4lDir.exists()) {
        for (const QString& vid : v4lDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (vid == "video0" && info.piCamera) continue;
            QFileInfo fi("/sys/class/video4linux/" + vid);
            const QString resolved = fi.isSymLink() ? fi.symLinkTarget() : fi.absoluteFilePath();
            if (resolved.contains("usb", Qt::CaseInsensitive)) {
                info.usbCameras.append("/dev/" + vid);
                qCInfo(hardwareDetect) << "USB camera: /dev/" + vid;
            }
        }
    }
}

// ── GPS ───────────────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectGps(HardwareInfo& info)
{
    // USB GPS: ttyUSB devices with USB subsystem
    QDir ttyDir("/sys/class/tty/");
    if (ttyDir.exists()) {
        for (const QString& tty : ttyDir.entryList(QStringList{"ttyUSB*"},
                                                    QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString subsysLink = "/sys/class/tty/" + tty + "/device/subsystem";
            QFileInfo fi(subsysLink);
            if (fi.isSymLink() && fi.symLinkTarget().contains("usb", Qt::CaseInsensitive)) {
                info.gpsPresent = true;
                info.gpsDevice  = "/dev/" + tty;
                qCInfo(hardwareDetect) << "GPS (USB):" << info.gpsDevice;
                break; // use first found
            }
        }
    }
    // Serial GPS device presence is confirmed in detect() after HAT probing
}

// ── HATs ──────────────────────────────────────────────────────────────────────

void RpiHardwareDetector::detectHats(HardwareInfo& info)
{
    // 1. Check HAT EEPROM first (preferred — vendor/product strings are definitive)
    detectHatEeprom(info);

    // 2. Probe I2C bus 1 for well-known HAT addresses
    if (!QFile::exists("/dev/i2c-1")) {
        qCWarning(hardwareDetect) << "I2C bus not available (/dev/i2c-1 missing) — skipping HAT probe";
        return;
    }

    if (!info.hatIqaudioDac) {
        info.hatIqaudioDac = probeI2cAddress(1, 0x4C);
        if (info.hatIqaudioDac) {
            info.iqaudioDac = true;
            qCInfo(hardwareDetect) << "IQAudio DAC+: detected via I2C 0x4C";
        } else {
            qCInfo(hardwareDetect) << "IQAudio DAC+: not detected";
        }
    }

    if (!info.hatCanBus) {
        info.hatCanBus = probeI2cAddress(1, 0x2E);
        qCInfo(hardwareDetect) << "CAN bus HAT:" << (info.hatCanBus ? "detected via I2C 0x2E" : "not detected");
    }

    if (!info.hatRtc) {
        info.hatRtc = probeI2cAddress(1, 0x68);
        qCInfo(hardwareDetect) << "RTC DS3231:" << (info.hatRtc ? "detected via I2C 0x68" : "not detected");
    }

    if (!info.hatGps) {
        info.hatGps = probeI2cAddress(1, 0x42);
        qCInfo(hardwareDetect) << "GPS HAT:" << (info.hatGps ? "detected via I2C 0x42" : "not detected");
    }
}

void RpiHardwareDetector::detectHatEeprom(HardwareInfo& info)
{
    const QString hatDir("/proc/device-tree/hat/");
    if (!QDir(hatDir).exists()) return;

    QFile vendorFile(hatDir + "vendor");
    QFile productFile(hatDir + "product");

    QString vendor, product;
    if (vendorFile.open(QIODevice::ReadOnly))
        vendor = QString::fromUtf8(vendorFile.readAll()).trimmed().toLower();
    if (productFile.open(QIODevice::ReadOnly))
        product = QString::fromUtf8(productFile.readAll()).trimmed().toLower();

    if (vendor.isEmpty() && product.isEmpty()) return;
    qCInfo(hardwareDetect) << "HAT EEPROM vendor:" << vendor << "product:" << product;

    if (vendor.contains("iqaudio") || product.contains("iqaudio") || product.contains("dac")) {
        info.hatIqaudioDac = true;
        info.iqaudioDac    = true;
        qCInfo(hardwareDetect) << "IQAudio DAC+: detected via HAT EEPROM";
    }
    if (product.contains("can")) {
        info.hatCanBus = true;
        qCInfo(hardwareDetect) << "CAN bus HAT: detected via HAT EEPROM";
    }
    if (product.contains("rtc")) {
        info.hatRtc = true;
        qCInfo(hardwareDetect) << "RTC: detected via HAT EEPROM";
    }
    if (product.contains("gps") || product.contains("neo") || product.contains("ublox")) {
        info.hatGps = true;
        qCInfo(hardwareDetect) << "GPS HAT: detected via HAT EEPROM";
    }
}

// ── I2C probe ─────────────────────────────────────────────────────────────────

bool RpiHardwareDetector::probeI2cAddress(int bus, quint8 address)
{
    const QByteArray devPath = QString("/dev/i2c-%1").arg(bus).toLocal8Bit();
    int fd = ::open(devPath.constData(), O_RDWR);
    if (fd < 0) {
        qCWarning(hardwareDetect) << "Cannot open" << devPath.constData()
                                  << ":" << strerror(errno);
        return false;
    }

    if (::ioctl(fd, I2C_SLAVE, static_cast<int>(address)) < 0) {
        ::close(fd);
        return false;
    }

    char byte;
    const bool present = (::read(fd, &byte, 1) >= 0);
    ::close(fd);
    return present;
}

} // namespace f1x::openauto::autoapp::Hardware
