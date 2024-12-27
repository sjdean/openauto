#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <QInputDevice>
#include <QSettings>
#include <QRandomGenerator>

namespace f1x::openauto::autoapp::configuration {

  Configuration::Configuration() {
    this->load();
  }

  QString generateRandomString(size_t length) {
    // Character set including uppercase, lowercase, and digits
    QString charset = QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    QString randomString;

    // Use Qt's random number generator
    for (size_t i = 0; i < length; ++i) {
      int randomIndex = QRandomGenerator::global()->bounded(charset.length());
      randomString.append(charset.at(randomIndex));
    }

    return randomString;
  }

  void Configuration::load() {
    boost::property_tree::ptree iniConfig;

    try {
      QSettings settings("journey.conf", QSettings::IniFormat);
      settings.beginGroup("Car");
      carMake_ = settings.value("Make", "Unknown").toString();
      carModel_ = settings.value("Model", "Unknown").toString();
      fuelType_ = static_cast<aap_protobuf::service::sensorsource::message::FuelType>(settings.value("FuelType",
                                                                                                     0).toInt());
      electricChargingType_ = static_cast<aap_protobuf::service::sensorsource::message::EvConnectorType>(settings.value(
          "ElectricChargingType", 0).toInt());
      driverPosition_ = static_cast<aap_protobuf::service::control::message::DriverPosition>(settings.value(
          "DriverPosition", 1).toInt());
      settings.endGroup();

      settings.beginGroup("Screen");
      brightnessDayMin_ = settings.value("DayMin", 255).toInt();
      brightnessDayMax_ = settings.value("DayMax", 255).toInt();
      brightnessNightMin_ = settings.value("DayMin", 255).toInt();
      brightnessNightMax_ = settings.value("DayMax", 255).toInt();
      brightnessCurrent_ = settings.value("Brightness", 255).toInt();
      dpi_ = settings.value("DPI", 140).toInt();
      settings.endGroup();

      settings.beginGroup("Video");
      margin_ = QRect(0,0, settings.value("Height", 0).toInt(), settings.value("Width", 0).toInt());

      omxLayer_ = settings.value("OMXLayer", 1).toInt();
      rotateDisplay_ = settings.value("Rotate", false).toBool();
      videoType_ = settings.value("Type", 1).toInt();
      settings.endGroup();

      settings.beginGroup("Audio");
      volumePlaybackMin_ = settings.value("PlaybackMin", 0).toInt();
      volumePlaybackMax_ = settings.value("PlaybackMax", 255).toInt();
      volumeCaptureMin_ = settings.value("CaptureMin", 0).toInt();
      volumeCaptureMax_ = settings.value("CaptureMax", 255).toInt();
      volumePlaybackCurrent_ = settings.value("Volume", 0).toInt();
      volumeCaptureCurrent_ = settings.value("Volume", 0).toInt();
      audioType_ = static_cast<AudioOutputBackendType>(settings.value("Type", 1).toInt());
      settings.endGroup();

      settings.beginGroup("Media");
      autoPlayback_ = settings.value("AutoPlayback", false).toBool();
      autoStart_ = settings.value("AutoStart", false).toBool();
      settings.endGroup();

      settings.beginGroup("AndroidAuto");
      channelMedia_ = settings.value("Media", true).toBool();
      channelGuidance_ = settings.value("Guidance", true).toBool();
      channelTelephony_ = settings.value("Telephony", true).toBool();
      videoFPS_ = static_cast<aap_protobuf::service::media::sink::message::VideoFrameRateType>(settings.value("FPS",1).toInt());
      videoResolution_ = static_cast<aap_protobuf::service::media::sink::message::VideoCodecResolutionType>(settings.value(
          "Resolution", true).toInt());
      settings.endGroup();

      settings.beginGroup("Bluetooth");
      bluetoothEnabled_ = settings.value("Enabled", true).toBool();
      bluetoothAdapterAddress_ = settings.value("BSSID", "").toString();
      settings.endGroup();

      settings.beginGroup("Wireless");
      wifiEnabled_ = settings.value("Enabled", true).toBool();
      wifiSsid_ = settings.value("SSID", "JourneyOS").toString();
      wifiPassword_ = settings.value("Password", generateRandomString(8)).toString();
      settings.endGroup();

      // Input Device
      // Output Device
      // Bluetooth Adapter
      // Enable Bluetooth
      // Bluetooth Pairing
      // Bluetooth Device
      // Paired Bluetooth

      // WiFi Host
      // Paired WiFi Device
      // Bluetooth Password
      // Driving Position

    }
    catch (const boost::property_tree::ini_parser_error &e) {
      OPENAUTO_LOG(warning) << "[Configuration] failed to read configuration file: " << cConfigFileName
                            << ", error: " << e.what()
                            << ". Using default configuration.";
      this->reset();
    }
  }

  void Configuration::reset() {

  }

  void Configuration::save() {

  }

  QString Configuration::getCarMake() {
    return carMake_;
  }

  QString Configuration::getCarModel() {
    return carModel_;
  }

  aap_protobuf::service::sensorsource::message::FuelType Configuration::getFuelType() {
    return fuelType_;
  }

  aap_protobuf::service::sensorsource::message::EvConnectorType Configuration::getElectricChargingType() {
    return electricChargingType_;
  }

  uint Configuration::getBrightnessDayMin() {
    return brightnessDayMin_;
  }

  uint Configuration::getBrightnessDayMax() {
    return brightnessDayMax_;
  }

  uint Configuration::getBrightnessNightMin() {
    return brightnessNightMin_;
  }

  uint Configuration::getBrightnessNightMax() {
    return brightnessNightMax_;
  }

  uint Configuration::getBrightnessCurrent() {
    return brightnessCurrent_;
  }

  uint Configuration::getDPI() {
    return dpi_;
  }

  QRect Configuration::getMargin() {
    return margin_;
  }

  int Configuration::getOMXLayer() {
    return omxLayer_;
  }

  bool Configuration::getRotateDisplay() {
    return rotateDisplay_;
  }

  int Configuration::getVideoType() {
    return videoType_;
  }

  uint Configuration::getVolumePlaybackMin() {
    return volumePlaybackMin_;
  }

  uint Configuration::getVolumePlaybackMax() {
    return volumePlaybackMax_;
  }

  uint Configuration::getVolumePlaybackCurrent() {
    return volumePlaybackCurrent_;
  }

  uint Configuration::getVolumeCaptureMin() {
    return volumeCaptureMin_;
  }

  uint Configuration::getVolumeCaptureMax() {
    return volumeCaptureMax_;
  }

  uint Configuration::getVolumeCaptureCurrent() {
    return volumeCaptureCurrent_;
  }

  AudioOutputBackendType Configuration::getAudioType() {
    return audioType_;
  }

  bool Configuration::getAutoStart() {
    return autoStart_;
  }

  bool Configuration::getAutoPlayback() {
    return autoPlayback_;
  }

  bool Configuration::getChannelMedia() {
    return channelMedia_;
  }

  bool Configuration::getChannelGuidance() {
    return channelGuidance_;
  }

  bool Configuration::getChannelTelephony() {
    return channelTelephony_;
  }

  aap_protobuf::service::media::sink::message::VideoFrameRateType Configuration::getFrameRate() {
    return videoFPS_;
  }

  aap_protobuf::service::media::sink::message::VideoCodecResolutionType Configuration::getVideoResolution() {
    return videoResolution_;
  }

  aap_protobuf::service::control::message::DriverPosition Configuration::getDriverPosition() {
    return driverPosition_;
  }

  bool Configuration::hasTouchScreen() const {
    auto touchdevs = QInputDevice::devices();

    OPENAUTO_LOG(info) << "[Configuration::hasTouchScreen] " << "Querying available touch devices ["
                       << touchdevs.length() << " available]";

    for (int i = 0; i < touchdevs.length(); i++) {
      if (touchdevs[i]->type() == QInputDevice::DeviceType::TouchScreen) {
        OPENAUTO_LOG(info) << "[Configuration::hasTouchScreen] Device " << i << ": "
                           << touchdevs[i]->name().toStdString();
        return true;
      }
    }
    return false;


  }




}



