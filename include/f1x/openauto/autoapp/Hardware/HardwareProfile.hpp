#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace f1x::openauto::autoapp::Hardware {

class HardwareProfile : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool        hasDSI            READ hasDSI            CONSTANT)
    Q_PROPERTY(bool        hasHDMI           READ hasHDMI           CONSTANT)
    Q_PROPERTY(QString     primaryDisplay    READ primaryDisplay    CONSTANT)
    Q_PROPERTY(bool        hasIQAudioDAC     READ hasIQAudioDAC     CONSTANT)
    Q_PROPERTY(bool        hasOnboardAudio   READ hasOnboardAudio   CONSTANT)
    Q_PROPERTY(QStringList usbAudioDevices   READ usbAudioDevices   CONSTANT)
    Q_PROPERTY(QStringList usbMicrophones    READ usbMicrophones    CONSTANT)
    Q_PROPERTY(bool        hasWifi           READ hasWifi           CONSTANT)
    Q_PROPERTY(bool        hasBluetooth      READ hasBluetooth      CONSTANT)
    Q_PROPERTY(bool        hasPiCamera       READ hasPiCamera       CONSTANT)
    Q_PROPERTY(QStringList usbCameras        READ usbCameras        CONSTANT)
    Q_PROPERTY(bool        hasGPS            READ hasGPS            CONSTANT)
    Q_PROPERTY(QString     gpsDevice         READ gpsDevice         CONSTANT)
    Q_PROPERTY(bool        hasCANBus         READ hasCANBus         CONSTANT)
    Q_PROPERTY(bool        hasRTC            READ hasRTC            CONSTANT)

public:
    static HardwareProfile* instance();
    static QString journeyOsRuntimePath(const QString& filename);

    bool        hasDSI()          const { return m_hasDSI; }
    bool        hasHDMI()         const { return m_hasHDMI; }
    QString     primaryDisplay()  const { return m_primaryDisplay; }
    bool        hasIQAudioDAC()   const { return m_hasIQAudioDAC; }
    bool        hasOnboardAudio() const { return m_hasOnboardAudio; }
    QStringList usbAudioDevices() const { return m_usbAudioDevices; }
    QStringList usbMicrophones()  const { return m_usbMicrophones; }
    bool        hasWifi()         const { return m_hasWifi; }
    bool        hasBluetooth()    const { return m_hasBluetooth; }
    bool        hasPiCamera()     const { return m_hasPiCamera; }
    QStringList usbCameras()      const { return m_usbCameras; }
    bool        hasGPS()          const { return m_hasGPS; }
    QString     gpsDevice()       const { return m_gpsDevice; }
    bool        hasCANBus()       const { return m_hasCANBus; }
    bool        hasRTC()          const { return m_hasRTC; }

private:
    explicit HardwareProfile(QObject* parent = nullptr);
    void load();

    bool        m_hasDSI{false};
    bool        m_hasHDMI{true};
    QString     m_primaryDisplay{"hdmi"};
    bool        m_hasIQAudioDAC{false};
    bool        m_hasOnboardAudio{false};
    QStringList m_usbAudioDevices;
    QStringList m_usbMicrophones;
    bool        m_hasWifi{true};
    bool        m_hasBluetooth{true};
    bool        m_hasPiCamera{false};
    QStringList m_usbCameras;
    bool        m_hasGPS{false};
    QString     m_gpsDevice;
    bool        m_hasCANBus{false};
    bool        m_hasRTC{false};
};

} // namespace f1x::openauto::autoapp::Hardware
