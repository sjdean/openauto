#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/SettingsView.hpp>

namespace f1x::openauto::autoapp::UI {
  SettingsView::SettingsView(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration,
                             QObject *parent) :
      QObject(parent),
      configuration_(std::move(configuration)),
      m_carMake(configuration_->getSettingByName<QString>("Car", "Make")),
      m_carModel(configuration_->getSettingByName<QString>("Car", "Model")),
      m_carFuelType(
          static_cast<aap_protobuf::service::sensorsource::message::FuelType>(configuration_->getSettingByName<int>(
              "Car",
              "FuelType"))),
      m_carEvConnectorType(
          static_cast<aap_protobuf::service::sensorsource::message::EvConnectorType>(configuration_->getSettingByName<int>(
              "Car", "EvConnectorType"))),
      m_carDriverPosition(
          static_cast<aap_protobuf::service::control::message::DriverPosition>(configuration_->getSettingByName<int>(
              "Car", "DriverPosition"))),

      m_screenBrightnessDayMin(configuration_->getSettingByName<int>("Screen", "DayMin")),
      m_screenBrightnessDayMax(configuration_->getSettingByName<int>("Screen", "DayMax")),
      m_screenBrightnessNightMin(configuration_->getSettingByName<int>("Screen", "NightMin")),
      m_screenBrightnessNightMax(configuration_->getSettingByName<int>("Screen", "NightMax")),
      m_screenBrightness(configuration_->getSettingByName<int>("Screen", "Brightness")),
      m_screenDPI(configuration_->getSettingByName<int>("Screen", "DPI")),

      m_videoMarginHeight(configuration_->getSettingByName<int>("Video", "Height")),
      m_videoMarginWidth(configuration_->getSettingByName<int>("Video", "Width")),
      m_videoOMXLayer(configuration_->getSettingByName<int>("Video", "OMXLayer")),
      m_videoType(configuration_->getSettingByName<int>("Video", "Type")),
      m_videoRotateDisplay(configuration_->getSettingByName<bool>("Video", "Rotate")),

      m_mediaAutoPlayback(configuration_->getSettingByName<bool>("Media", "AutoPlayback")),
      m_mediaAutoStart(configuration_->getSettingByName<bool>("Media", "AutoStart")),

      m_aaChannelMedia(configuration_->getSettingByName<bool>("AndroidAuto", "Media")),
      m_aaChannelGuidance(configuration_->getSettingByName<bool>("AndroidAuto", "Guidance")),
      m_aaChannelTelephony(configuration_->getSettingByName<bool>("AndroidAuto", "Telephony")),

      m_audioVolumePlaybackMin(configuration_->getSettingByName<int>("Audio", "PlaybackMin")),
      m_audioVolumePlaybackMax(configuration_->getSettingByName<int>("Audio", "PlaybackMax")),
      m_audioVolumeCaptureMin(configuration_->getSettingByName<int>("Audio", "CaptureMin")),
      m_audioVolumeCaptureMax(configuration_->getSettingByName<int>("Audio", "CaptureMax")),
      m_audioVolumePlayback(configuration_->getSettingByName<int>("Audio", "PlaybackVolume")),
      m_audioVolumeCapture(configuration_->getSettingByName<int>("Audio", "CaptureVolume")),

      m_audioType(
          static_cast<f1x::openauto::autoapp::configuration::AudioOutputBackendType>(configuration_->getSettingByName<int>(
              "Audio", "Type"))) {

  }

  QString SettingsView::getCarMake() const {
    return m_carMake;
  }

  void SettingsView::setCarMake(QString value) {
    if (m_carMake != value) {
      m_carMake = value;
      emit carMakeChanged();
    }
  }

  QString SettingsView::getCarModel() const {
    return m_carModel;
  }

  void SettingsView::setCarModel(QString value) {
    if (m_carModel != value) {
      m_carModel = value;
      emit carModelChanged();
    }
  }

  bool SettingsView::getMediaAutoPlayback() const {
    return m_mediaAutoPlayback;
  }

  void SettingsView::setMediaAutoPlayback(bool value) {
    if (m_mediaAutoPlayback != value) {
      m_mediaAutoPlayback = value;
      emit mediaAutoPlaybackChanged();
    }
  }

  bool SettingsView::getMediaAutoStart() const {
    return m_mediaAutoStart;
  }

  void SettingsView::setMediaAutoStart(bool value) {
    if (m_mediaAutoStart != value) {
      m_mediaAutoStart = value;
      emit mediaAutoStartChanged();
    }
  }

  bool SettingsView::getAAChannelMedia() const {
    return m_aaChannelMedia;
  }

  void SettingsView::setAAChannelMedia(bool value) {
    if (m_aaChannelMedia != value) {
      m_aaChannelMedia = value;
      emit aaChannelMediaChanged();
    }
  }

  bool SettingsView::getAAChannelGuidance() const {
    return m_aaChannelGuidance;
  }

  void SettingsView::setAAChannelGuidance(bool value) {
    if (m_aaChannelGuidance != value) {
      m_aaChannelGuidance = value;
      emit aaChannelGuidanceChanged();
    }
  }

  bool SettingsView::getAAChannelTelephony() const {
    return m_aaChannelTelephony;
  }

  void SettingsView::setAAChannelTelephony(bool value) {
    if (m_aaChannelTelephony != value) {
      m_aaChannelTelephony = value;
      emit aaChannelTelephonyChanged();
    }
  }

  int SettingsView::getVideoOMXLayer() const {
    return m_videoOMXLayer;
  }

  void SettingsView::setVideoOMXLayer(int value) {
    if (value != m_videoOMXLayer) {
      m_videoOMXLayer = value;
      emit videoOMXLayerChanged();
    }
  }

  int SettingsView::getVideoMarginHeight() const {
    return m_videoMarginHeight;
  }

  void SettingsView::setVideoMarginHeight(int value) {
    if (value != m_videoMarginHeight) {
      m_videoMarginHeight = value;
      emit videoMarginHeightChanged();
    }
  }

  int SettingsView::getVideoMarginWidth() const {
    return m_videoMarginWidth;
  }

  void SettingsView::setVideoMarginWidth(int value) {
    if (value != m_videoMarginWidth) {
      m_videoMarginWidth = value;
      emit videoMarginWidthChanged();
    }
  }

  int SettingsView::getAudioVolumePlayback() const {
    return m_audioVolumePlayback;
  }

  void SettingsView::setAudioVolumePlayback(int value) {
    if (value != m_audioVolumePlayback) {
      m_audioVolumePlayback = value;
      emit audioVolumePlaybackChanged();
    }
  }

  int SettingsView::getAudioVolumeCapture() const {
    return m_audioVolumeCapture;
  }

  void SettingsView::setAudioVolumeCapture(int value) {
    if (value != m_audioVolumeCapture) {
      m_audioVolumeCapture = value;
      emit audioVolumeCaptureChanged();
    }
  }

  int SettingsView::getScreenBrightnessDayMin() const {
    return m_screenBrightnessDayMin;
  }

  void SettingsView::setScreenBrightnessDayMin(int value) {
    if (value != m_screenBrightnessDayMin) {
      m_screenBrightnessDayMin = value;
      emit screenBrightnessDayMinChanged();
    }
  }

  int SettingsView::getScreenBrightnessDayMax() const {
    return m_screenBrightnessDayMax;
  }

  void SettingsView::setScreenBrightnessDayMax(int value) {
    if (value != m_screenBrightnessDayMax) {
      m_screenBrightnessDayMax = value;
      emit screenBrightnessDayMaxChanged();
    }
  }

  int SettingsView::getScreenBrightnessNightMin() const {
    return m_screenBrightnessNightMin;
  }

  void SettingsView::setScreenBrightnessNightMin(int value) {
    if (value != m_screenBrightnessNightMin) {
      configuration_->updateSettingByName<int>("Screen", "NightMin", value);
      m_screenBrightnessNightMin = value;
      emit screenBrightnessNightMinChanged();
    }
  }

  int SettingsView::getScreenBrightnessNightMax() const {
    return m_screenBrightnessNightMax;
  }

  void SettingsView::setScreenBrightnessNightMax(int value) {
    if (value != m_screenBrightnessNightMax) {
      m_screenBrightnessNightMax = value;
      emit screenBrightnessNightMaxChanged();
    }
  }

  bool SettingsView::getVideoRotateDisplay() const {
    return m_videoRotateDisplay;
  }

  void SettingsView::setVideoRotateDisplay(bool value) {
    if (m_videoRotateDisplay != value) {
      m_videoRotateDisplay = value;
      emit videoRotateDisplayChanged();
    }
  }

  int SettingsView::getScreenDPI() const {
    return m_screenDPI;
  }

  void SettingsView::setScreenDPI(int value) {
    if (value != m_screenDPI) {
      m_screenDPI = value;
      emit screenDPIChanged();
    }
  }

  void SettingsView::setAudioType(f1x::openauto::autoapp::configuration::AudioOutputBackendType value) {
    if (value != m_audioType) {
      m_audioType = value;
      emit audioTypeChanged(value);
    }
  }

  void SettingsView::setAudioVolumeCaptureMax(int value) {
    if (value != m_audioVolumeCaptureMax) {
      m_audioVolumeCaptureMax = value;
      emit audioVolumeCaptureMaxChanged();
    }
  }

  void SettingsView::setAudioVolumeCaptureMin(int value) {
    if (value != m_audioVolumeCaptureMin) {
      m_audioVolumeCaptureMin = value;
      emit audioVolumeCaptureMinChanged();
    }
  }

  void SettingsView::setAudioVolumePlaybackMax(int value) {
    if (value != m_audioVolumePlaybackMax) {
      m_audioVolumePlaybackMax = value;
      emit audioVolumePlaybackMaxChanged();
    }
  }

  void SettingsView::setAudioVolumePlaybackMin(int value) {
    if (value != m_audioVolumePlaybackMin) {
      m_audioVolumePlaybackMin = value;
      emit audioVolumePlaybackMinChanged();
    }
  }

  f1x::openauto::autoapp::configuration::AudioOutputBackendType SettingsView::getAudioType() const {
    return m_audioType;
  }

  int SettingsView::getAudioVolumeCaptureMax() const {
    return m_audioVolumeCaptureMax;
  }

  int SettingsView::getAudioVolumeCaptureMin() const {
    return m_audioVolumeCaptureMin;
  }

  int SettingsView::getAudioVolumePlaybackMax() const {
    return m_audioVolumePlaybackMax;
  }

  int SettingsView::getAudioVolumePlaybackMin() const {
    return m_audioVolumePlaybackMin;
  }

  void SettingsView::setVideoType(int value) {
    if (value != m_videoType) {
      m_videoType = value;
      emit videoTypeChanged(value);
    }
  }

  int SettingsView::getVideoType() const {
    return m_videoType;
  }

  void SettingsView::setScreenBrightness(int value) {
    if (value != m_screenBrightness) {
      m_screenBrightness = value;
      emit screenBrightnessChanged();
    }
  }

  int SettingsView::getScreenBrightness() const {
    return m_screenBrightness;
  }

  void SettingsView::setCarDriverPosition(aap_protobuf::service::control::message::DriverPosition value) {
    if (value != m_carDriverPosition) {
      m_carDriverPosition = value;
      emit carDriverPositionChanged();
    }
  }

  void SettingsView::setCarEvConnectorType(aap_protobuf::service::sensorsource::message::EvConnectorType value) {
    if (value != m_carEvConnectorType) {
      m_carEvConnectorType = value;
      emit carEvConnectorTypeChanged();
    }
  }

  void SettingsView::setCarFuelType(aap_protobuf::service::sensorsource::message::FuelType value) {
    if (value != m_carFuelType) {
      m_carFuelType = value;
      emit carFuelTypeChanged();
    }
  }

  aap_protobuf::service::control::message::DriverPosition SettingsView::getCarDriverPosition() const {
    return m_carDriverPosition;
  }

  aap_protobuf::service::sensorsource::message::EvConnectorType SettingsView::getCarEvConnectorType() const {
    return m_carEvConnectorType;
  }

  aap_protobuf::service::sensorsource::message::FuelType SettingsView::getCarFuelType() const {
    return m_carFuelType;
  }
}