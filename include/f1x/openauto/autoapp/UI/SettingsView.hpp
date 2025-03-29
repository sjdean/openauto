#ifndef OPENAUTO_SETTINGSVIEW_H
#define OPENAUTO_SETTINGSVIEW_H

#include <QtCore/QObject>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::UI {

  class SettingsView : public QObject {
    Q_OBJECT

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
    Q_PROPERTY(int videoOMXLayer READ getVideoOMXLayer WRITE setVideoOMXLayer NOTIFY videoOMXLayerChanged)
    Q_PROPERTY(int videoType READ getVideoType WRITE setVideoType NOTIFY videoTypeChanged)
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
    Q_PROPERTY(f1x::openauto::autoapp::configuration::AudioOutputBackendType audioType READ getAudioType WRITE setAudioType NOTIFY audioTypeChanged)
    Q_PROPERTY(
        int audioPlaybackDevice READ getAudioPlaybackDeviceCapture WRITE setAudioPlaybackDeviceCapture NOTIFY audioPlaybackDeviceChanged)
    Q_PROPERTY(
        int audioCaptureDevice READ getAudioCaptureDeviceCapture WRITE setAudioCaptureDeviceCapture NOTIFY audioCaptureDeviceChanged)

  public:
    explicit SettingsView(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration,
                          QObject *parent = nullptr);

  signals:

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

    void videoOMXLayerChanged();

    void videoTypeChanged(int type);

    void videoRotateDisplayChanged();

    void mediaAutoPlaybackChanged();

    void mediaAutoStartChanged();

    void aaChannelMediaChanged();

    void aaChannelGuidanceChanged();

    void aaChannelTelephonyChanged();

    void aaFrameRateChanged();

    void aaResolutionChanged();

    void audioVolumePlaybackMinChanged();

    void audioVolumePlaybackMaxChanged();

    void audioVolumePlaybackChanged();

    void audioVolumeCaptureChanged();

    void audioVolumeCaptureMinChanged();

    void audioVolumeCaptureMaxChanged();

    void audioTypeChanged(f1x::openauto::autoapp::configuration::AudioOutputBackendType type);

    void audioPlaybackDeviceChanged();
    void audioCaptureDeviceChanged();

  private:
    f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;

    // Car Settings
    QString m_carMake;
    QString m_carModel;
    aap_protobuf::service::sensorsource::message::FuelType m_carFuelType;
    aap_protobuf::service::sensorsource::message::EvConnectorType m_carEvConnectorType;
    aap_protobuf::service::control::message::DriverPosition m_carDriverPosition;

    QString getCarMake() const;

    QString getCarModel() const;

    aap_protobuf::service::sensorsource::message::FuelType getCarFuelType() const;

    aap_protobuf::service::sensorsource::message::EvConnectorType getCarEvConnectorType() const;

    aap_protobuf::service::control::message::DriverPosition getCarDriverPosition() const;

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
    int m_videoOMXLayer;
    int m_videoType;
    bool m_videoRotateDisplay;

    int getVideoMarginHeight() const;

    int getVideoMarginWidth() const;

    int getVideoOMXLayer() const;

    int getVideoType() const;

    bool getVideoRotateDisplay() const;

    void setVideoMarginHeight(int value);

    void setVideoMarginWidth(int value);

    void setVideoOMXLayer(int value);

    void setVideoType(int value);

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

    f1x::openauto::autoapp::configuration::AudioOutputBackendType m_audioType;

    int getAudioVolumePlaybackMin() const;

    int getAudioVolumePlaybackMax() const;

    int getAudioVolumeCaptureMin() const;

    int getAudioVolumeCaptureMax() const;

    int getAudioVolumePlayback() const;

    int getAudioVolumeCapture() const;

    f1x::openauto::autoapp::configuration::AudioOutputBackendType getAudioType() const;

    QString getAudioPlaybackDeviceCapture() const;
    QString getAudioCaptureDeviceCapture() const;

    void setAudioVolumePlaybackMin(int value);

    void setAudioVolumePlaybackMax(int value);

    void setAudioVolumeCaptureMin(int value);

    void setAudioVolumeCaptureMax(int value);

    void setAudioVolumePlayback(int value);

    void setAudioVolumeCapture(int value);

    void setAudioType(f1x::openauto::autoapp::configuration::AudioOutputBackendType value);

    void setAudioPlaybackDeviceCapture(QString value);
    void setAudioCaptureDeviceCapture(QString value);
  };
}
#endif //OPENAUTO_SETTINGSVIEW_H
