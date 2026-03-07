#pragma once

#include <QtCore/QObject>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {

  class SettingsViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString carId READ getCarId WRITE setCarId NOTIFY carIdChanged)
    Q_PROPERTY(QString carMake READ getCarMake WRITE setCarMake NOTIFY carMakeChanged)
    Q_PROPERTY(QString carModel READ getCarModel WRITE setCarModel NOTIFY carModelChanged)
    Q_PROPERTY(aap_protobuf::service::sensorsource::message::FuelType carFuelType READ getCarFuelType WRITE setCarFuelType NOTIFY carFuelTypeChanged)
    Q_PROPERTY(
        aap_protobuf::service::sensorsource::message::EvConnectorType carEvConnectorType READ getCarEvConnectorType WRITE setCarEvConnectorType NOTIFY carEvConnectorTypeChanged)
    Q_PROPERTY(
        aap_protobuf::service::control::message::DriverPosition carDriverPosition READ getCarDriverPosition WRITE setCarDriverPosition NOTIFY carDriverPositionChanged)

    Q_PROPERTY(
        int screenBrightnessDayMin READ getScreenBrightnessDayMin WRITE setScreenBrightnessDayMin NOTIFY screenBrightnessDayMinChanged)
    Q_PROPERTY(
        int screenBrightnessDayMax READ getScreenBrightnessDayMax WRITE setScreenBrightnessDayMax NOTIFY screenBrightnessDayMaxChanged)
    Q_PROPERTY(
        int screenBrightnessNightMin READ getScreenBrightnessNightMin WRITE setScreenBrightnessNightMin NOTIFY screenBrightnessNightMinChanged)
    Q_PROPERTY(
        int screenBrightnessNightMax READ getScreenBrightnessNightMax WRITE setScreenBrightnessNightMax NOTIFY screenBrightnessNightMaxChanged)
    Q_PROPERTY(int screenBrightness READ getScreenBrightness WRITE setScreenBrightness NOTIFY screenBrightnessChanged)
    Q_PROPERTY(int screenDPI READ getScreenDPI WRITE setScreenDPI NOTIFY screenDPIChanged)

    Q_PROPERTY(
        int videoMarginHeight READ getVideoMarginHeight WRITE setVideoMarginHeight NOTIFY videoMarginHeightChanged)
    Q_PROPERTY(int videoMarginWidth READ getVideoMarginWidth WRITE setVideoMarginWidth NOTIFY videoMarginWidthChanged)

    Q_PROPERTY(
        bool videoRotateDisplay READ getVideoRotateDisplay WRITE setVideoRotateDisplay NOTIFY videoRotateDisplayChanged)

    Q_PROPERTY(
        bool mediaAutoPlayback READ getMediaAutoPlayback WRITE setMediaAutoPlayback NOTIFY mediaAutoPlaybackChanged)
    Q_PROPERTY(bool mediaAutoStart READ getMediaAutoStart WRITE setMediaAutoStart NOTIFY mediaAutoStartChanged)

    Q_PROPERTY(bool aaChannelMedia READ getAAChannelMedia WRITE setAAChannelMedia NOTIFY aaChannelMediaChanged)
    Q_PROPERTY(
        bool aaChannelGuidance READ getAAChannelGuidance WRITE setAAChannelGuidance NOTIFY aaChannelGuidanceChanged)
    Q_PROPERTY(
        bool aaChannelTelephony READ getAAChannelTelephony WRITE setAAChannelTelephony NOTIFY aaChannelTelephonyChanged)
    Q_PROPERTY(aap_protobuf::service::media::sink::message::VideoFrameRateType aaFrameRate READ getAAFrameRate WRITE setAAFrameRate NOTIFY aaFrameRateChanged)
    Q_PROPERTY(aap_protobuf::service::media::sink::message::VideoCodecResolutionType aaResolution READ getAAResolution WRITE setAAResolution NOTIFY aaResolutionChanged)
    Q_PROPERTY(
        int audioVolumePlaybackMin READ getAudioVolumePlaybackMin WRITE setAudioVolumePlaybackMin NOTIFY audioVolumePlaybackMinChanged)
    Q_PROPERTY(
        int audioVolumePlaybackMax READ getAudioVolumePlaybackMax WRITE setAudioVolumePlaybackMax NOTIFY audioVolumePlaybackMaxChanged)
    Q_PROPERTY(
        int audioVolumeCaptureMin READ getAudioVolumeCaptureMin WRITE setAudioVolumeCaptureMin NOTIFY audioVolumeCaptureMinChanged)
    Q_PROPERTY(
        int audioVolumeCaptureMax READ getAudioVolumeCaptureMax WRITE setAudioVolumeCaptureMax NOTIFY audioVolumeCaptureMaxChanged)
    Q_PROPERTY(
        int audioVolumePlayback READ getAudioVolumePlayback WRITE setAudioVolumePlayback NOTIFY audioVolumePlaybackChanged)
    Q_PROPERTY(
        int audioVolumeCapture READ getAudioVolumeCapture WRITE setAudioVolumeCapture NOTIFY audioVolumeCaptureChanged)
    Q_PROPERTY(
        QString audioPlaybackDevice READ getAudioPlaybackDevice WRITE setAudioPlaybackDevice NOTIFY audioPlaybackDeviceChanged)
    Q_PROPERTY(
        QString audioCaptureDevice READ getAudioCaptureDevice WRITE setAudioCaptureDevice NOTIFY audioCaptureDeviceChanged)
    Q_PROPERTY(
        QString wirelessClientSSID READ getWirelessClientSSID WRITE setWirelessClientSSID NOTIFY wirelessClientSSIDChanged)
    Q_PROPERTY (
        QString wirelessClientPassword READ getWirelessClientPassword WRITE setWirelessClientPassword NOTIFY wirelessClientPasswordChanged)
    Q_PROPERTY(
        QString wirelessHotspotSSID READ getWirelessHotspotSSID WRITE setWirelessHotspotSSID NOTIFY wirelessHotspotSSIDChanged)
    Q_PROPERTY (
        QString wirelessHotspotPassword READ getWirelessHotspotPassword WRITE setWirelessHotspotPassword NOTIFY wirelessHotspotPasswordChanged)
    Q_PROPERTY(common::Enum::WirelessType::Value wirelessType READ getWirelessType WRITE setWirelessType NOTIFY wirelessTypeChanged)
    Q_PROPERTY(bool wirelessEnabled READ getWirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
      Q_PROPERTY(QString wirelessHotspotInterface READ getWirelessHotspotInterface WRITE setWirelessHotspotInterface NOTIFY wirelessHotspotInterfaceChanged)

    Q_PROPERTY(QString hwBluetoothAdapter READ getHwBluetoothAdapter WRITE setHwBluetoothAdapter NOTIFY hwBluetoothAdapterChanged)
    Q_PROPERTY(bool headUnitMode READ isHeadUnitMode WRITE setHeadUnitMode NOTIFY headUnitModeChanged)
  public:
    explicit SettingsViewModel(configuration::IConfiguration::Pointer configuration,
                          QObject *parent = nullptr);

    bool isHeadUnitMode() const;

  signals:

    void carIdChanged();

    void carMakeChanged();

    void carModelChanged();

    void carEvConnectorTypeChanged();

    void carFuelTypeChanged();

    void carDriverPositionChanged();

    void screenBrightnessDayMinChanged();

    void screenBrightnessDayMaxChanged();

    void screenBrightnessNightMinChanged();

    void screenBrightnessNightMaxChanged();

    void screenBrightnessChanged();

    void screenDPIChanged();

    void videoMarginHeightChanged();

    void videoMarginWidthChanged();

    void videoRotateDisplayChanged();

    void mediaAutoPlaybackChanged();

    void mediaAutoStartChanged();

    void aaChannelMediaChanged();

    void aaChannelGuidanceChanged();

    void aaChannelTelephonyChanged();

    void aaFrameRateChanged();

    void aaResolutionChanged();

    void aaResolution();

    void audioVolumePlaybackMinChanged();

    void audioVolumePlaybackMaxChanged();

    void audioVolumePlaybackChanged();

    void audioVolumeCaptureChanged();

    void audioVolumeCaptureMinChanged();

    void audioVolumeCaptureMaxChanged();

    void audioPlaybackDeviceChanged();
    void audioCaptureDeviceChanged();

    void wirelessClientSSIDChanged();
    void wirelessClientPasswordChanged();
    void wirelessHotspotSSIDChanged();
    void wirelessHotspotPasswordChanged();

    void wirelessTypeChanged();
    void wirelessEnabledChanged();
      void wirelessHotspotInterfaceChanged();

    void hwBluetoothAdapterChanged();

    void headUnitModeChanged();

  private:
    configuration::IConfiguration::Pointer configuration_;

    // Car Settings
    QString m_carId;
    QString m_carMake;
    QString m_carModel;
    aap_protobuf::service::sensorsource::message::FuelType m_carFuelType;
    aap_protobuf::service::sensorsource::message::EvConnectorType m_carEvConnectorType;
    aap_protobuf::service::control::message::DriverPosition m_carDriverPosition;

    QString getCarId() const;

    QString getCarMake() const;

    QString getCarModel() const;

    aap_protobuf::service::sensorsource::message::FuelType getCarFuelType() const;

    aap_protobuf::service::sensorsource::message::EvConnectorType getCarEvConnectorType() const;

    aap_protobuf::service::control::message::DriverPosition getCarDriverPosition() const;

    void setCarId(QString value);

    void setCarMake(QString value);

    void setCarModel(QString value);

    void setCarFuelType(aap_protobuf::service::sensorsource::message::FuelType value);

    void setCarEvConnectorType(aap_protobuf::service::sensorsource::message::EvConnectorType value);

    void setCarDriverPosition(aap_protobuf::service::control::message::DriverPosition value);

    // Screen
    int m_screenBrightnessDayMin;
    int m_screenBrightnessDayMax;
    int m_screenBrightnessNightMin;
    int m_screenBrightnessNightMax;
    int m_screenBrightness;
    int m_screenDPI;

    int getScreenBrightnessDayMin() const;

    int getScreenBrightnessDayMax() const;

    int getScreenBrightnessNightMin() const;

    int getScreenBrightnessNightMax() const;

    int getScreenBrightness() const;

    int getScreenDPI() const;

    void setScreenBrightnessDayMin(int value);

    void setScreenBrightnessDayMax(int value);

    void setScreenBrightnessNightMin(int value);

    void setScreenBrightnessNightMax(int value);

    void setScreenBrightness(int value);

    void setScreenDPI(int value);

    // Video
    int m_videoMarginHeight;
    int m_videoMarginWidth;

    bool m_videoRotateDisplay;

    int getVideoMarginHeight() const;

    int getVideoMarginWidth() const;

    bool getVideoRotateDisplay() const;

    void setVideoMarginHeight(int value);

    void setVideoMarginWidth(int value);

    void setVideoRotateDisplay(bool value);

    // Media Settings
    bool m_mediaAutoPlayback;
    bool m_mediaAutoStart;

    bool getMediaAutoPlayback() const;

    bool getMediaAutoStart() const;

    void setMediaAutoPlayback(bool value);

    void setMediaAutoStart(bool value);

    // Android Auto Settings - Channels
    bool m_aaChannelMedia;
    bool m_aaChannelGuidance;
    bool m_aaChannelTelephony;
    aap_protobuf::service::media::sink::message::VideoFrameRateType m_aaFrameRate;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType m_aaResolution;

    bool getAAChannelMedia() const;

    bool getAAChannelGuidance() const;

    bool getAAChannelTelephony() const;

    aap_protobuf::service::media::sink::message::VideoFrameRateType getAAFrameRate() const;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType getAAResolution() const;

    void setAAChannelMedia(bool value);

    void setAAChannelGuidance(bool value);

    void setAAChannelTelephony(bool value);

    void setAAFrameRate(aap_protobuf::service::media::sink::message::VideoFrameRateType value);
    void setAAResolution(aap_protobuf::service::media::sink::message::VideoCodecResolutionType value);


    // Audio Settings
    int m_audioVolumePlaybackMin;
    int m_audioVolumePlaybackMax;
    int m_audioVolumeCaptureMin;
    int m_audioVolumeCaptureMax;
    int m_audioVolumePlayback;
    int m_audioVolumeCapture;
    QString m_audioCaptureDeviceValue;
    QString m_audioPlaybackDeviceValue;

    int getAudioVolumePlaybackMin() const;

    int getAudioVolumePlaybackMax() const;

    int getAudioVolumeCaptureMin() const;

    int getAudioVolumeCaptureMax() const;

    int getAudioVolumePlayback() const;

    int getAudioVolumeCapture() const;

    QString getAudioPlaybackDevice() const;
    QString getAudioCaptureDevice() const;

    void setAudioVolumePlaybackMin(int value);

    void setAudioVolumePlaybackMax(int value);

    void setAudioVolumeCaptureMin(int value);

    void setAudioVolumeCaptureMax(int value);

    void setAudioVolumePlayback(int value);

    void setAudioVolumeCapture(int value);

    void setAudioPlaybackDevice(QString value);
    void setAudioCaptureDevice(QString value);

    // Wireless Settings
    QString m_wirelessClientSSID;
    QString m_wirelessClientPassword;
    QString m_wirelessHotspotSSID;
    QString m_wirelessHotspotPassword;
    QString m_wirelessHotspotInterface;


    common::Enum::WirelessType::Value m_wirelessType;
    bool m_wirelessEnabled;

    void setWirelessClientSSID(QString value);
    void setWirelessClientPassword(QString value);
    void setWirelessHotspotSSID(QString value);
    void setWirelessHotspotPassword(QString value);
      void setWirelessHotspotInterface(QString value);
    void setWirelessType(common::Enum::WirelessType::Value value);
    void setWirelessEnabled(bool value);


    QString getWirelessClientSSID();
    QString getWirelessClientPassword();
    QString getWirelessHotspotSSID();
    QString getWirelessHotspotPassword();
      QString getWirelessHotspotInterface();

    common::Enum::WirelessType::Value getWirelessType();
    bool getWirelessEnabled();

    QString m_hwBluetoothAdapter;
    QString getHwBluetoothAdapter();
    void setHwBluetoothAdapter(QString value);

    bool m_headUnitMode;
    void setHeadUnitMode(bool value);

  };
}
