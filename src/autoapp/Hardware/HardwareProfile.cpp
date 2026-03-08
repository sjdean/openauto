#include "f1x/openauto/autoapp/Hardware/HardwareProfile.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(hardwareProfileLog, "journeyos.hardware")

namespace f1x::openauto::autoapp::Hardware {

HardwareProfile* HardwareProfile::instance()
{
    static HardwareProfile profile;
    return &profile;
}

QString HardwareProfile::journeyOsRuntimePath(const QString& filename)
{
#ifdef Q_OS_LINUX
    return "/run/journeyos/" + filename;
#else
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation)
           + "/journeyos/" + filename;
#endif
}

HardwareProfile::HardwareProfile(QObject* parent)
    : QObject(parent)
{
    load();
}

void HardwareProfile::load()
{
    const QString path = journeyOsRuntimePath("hardware.json");
    QFile file(path);

    if (!file.exists()) {
        qCWarning(hardwareProfileLog) << "hardware.json not found at" << path << "— using safe defaults";
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(hardwareProfileLog) << "Cannot read" << path << ":" << file.errorString() << "— using safe defaults";
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(hardwareProfileLog) << "hardware.json parse error:" << parseError.errorString() << "— using safe defaults";
        return;
    }
    if (!doc.isObject()) {
        qCWarning(hardwareProfileLog) << "hardware.json is not a JSON object — using safe defaults";
        return;
    }

    const QJsonObject root = doc.object();

    // Display
    const QJsonObject display = root["display"].toObject();
    m_primaryDisplay = display["primary"].toString("hdmi");
    m_hasDSI         = display["dsi_present"].toBool(false);
    m_hasHDMI        = display["hdmi_present"].toBool(true);
    qCInfo(hardwareProfileLog) << "Display: primary=" << m_primaryDisplay
                               << "dsi=" << m_hasDSI << "hdmi=" << m_hasHDMI;

    // Audio
    const QJsonObject audio = root["audio"].toObject();
    m_hasIQAudioDAC   = audio["iqaudio_dac"].toBool(false);
    m_hasOnboardAudio = audio["onboard"].toBool(false);
    for (const QJsonValue& v : audio["usb_audio"].toArray())
        m_usbAudioDevices.append(v.toString());
    for (const QJsonValue& v : audio["usb_microphone"].toArray())
        m_usbMicrophones.append(v.toString());
    qCInfo(hardwareProfileLog) << "Audio: iqaudio=" << m_hasIQAudioDAC
                               << "onboard=" << m_hasOnboardAudio;

    // Connectivity
    const QJsonObject connectivity = root["connectivity"].toObject();
    m_hasWifi      = connectivity["wifi_onboard"].toBool(true)
                   || connectivity["wifi_usb"].toBool(false);
    m_hasBluetooth = connectivity["bluetooth_onboard"].toBool(true)
                   || connectivity["bluetooth_usb"].toBool(false);
    qCInfo(hardwareProfileLog) << "Connectivity: wifi=" << m_hasWifi
                               << "bluetooth=" << m_hasBluetooth;

    // Camera
    const QJsonObject camera = root["camera"].toObject();
    m_hasPiCamera = camera["pi_camera"].toBool(false);
    for (const QJsonValue& v : camera["usb_cameras"].toArray())
        m_usbCameras.append(v.toString());

    // GPS
    const QJsonObject gps = root["gps"].toObject();
    m_hasGPS    = gps["present"].toBool(false);
    m_gpsDevice = gps["device"].toString();
    if (m_hasGPS) qCInfo(hardwareProfileLog) << "GPS: present at" << m_gpsDevice;

    // HATs
    const QJsonObject hats = root["hats"].toObject();
    m_hasIQAudioDAC = m_hasIQAudioDAC || hats["iqaudio_dac"].toBool(false);
    m_hasCANBus     = hats["can_bus"].toBool(false);
    m_hasRTC        = hats["rtc"].toBool(false);
    if (hats["gps_hat"].toBool(false)) m_hasGPS = true;
    qCInfo(hardwareProfileLog) << "HATs: iqaudio=" << m_hasIQAudioDAC
                               << "can=" << m_hasCANBus
                               << "rtc=" << m_hasRTC
                               << "gps=" << hats["gps_hat"].toBool(false);
}

} // namespace f1x::openauto::autoapp::Hardware
