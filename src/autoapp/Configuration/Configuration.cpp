#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <QInputDevice>
#include <QSettings>
#include <QRandomGenerator>
#include <service/control/message/DriverPosition.pb.h>
#include <service/media/sink/message/VideoFrameRateType.pb.h>
#include <service/sensorsource/message/EvConnectorType.pb.h>
#include <service/sensorsource/message/FuelType.pb.h>

#include "f1x/openauto/Common/Enum/AudioOutputType.hpp"
#include "f1x/openauto/Common/Enum/VideoType.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::configuration {

  Configuration::Configuration()
  : m_settings("journey.conf", QSettings::IniFormat)
  {
    m_configurationGroups.clear();

    // Initialise and Configure Default Settings prior to loading...
    ConfigurationGroup carGroup("Car");
    carGroup.addSetting<QString>("Make", "Unknown");
    carGroup.addSetting<QString>("Model", "Unknown");
    carGroup.addSetting<int>("FuelType", aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_UNKNOWN);
    carGroup.addSetting<int>("EvConnectorType", aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_UNKNOWN);
    carGroup.addSetting<int>("DriverPosition", aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
    carGroup.addSetting<QString>("Id", generateRandomString(128));
    carGroup.load(m_settings);
    m_configurationGroups.append(carGroup);

    ConfigurationGroup screenGroup("Screen");
    screenGroup.addSetting<int>("DayMin", 10);
    screenGroup.addSetting<int>("DayMax", 150);
    screenGroup.addSetting<int>("NightMin", 10);
    screenGroup.addSetting<int>("NightMax", 150);
    screenGroup.addSetting<int>("Brightness", 150);
    screenGroup.addSetting<int>("DPI", 140);
    screenGroup.load(m_settings);
    m_configurationGroups.append(screenGroup);

    ConfigurationGroup videoGroup("Video");
    videoGroup.addSetting<int>("Height", 0);
    videoGroup.addSetting<int>("Width", 0);
    videoGroup.addSetting<int>("OMXLayer", 2);
    videoGroup.addSetting<bool>("Rotate", false);
    videoGroup.addSetting<f1x::openauto::common::Enum::VideoType::Value>("Type", f1x::openauto::common::Enum::VideoType::Value::EGL);
    videoGroup.load(m_settings);
    m_configurationGroups.append(videoGroup);

    ConfigurationGroup audioGroup("Audio");
    audioGroup.addSetting<int>("PlaybackMin", 0);
    audioGroup.addSetting<int>("PlaybackMax", 255);
    audioGroup.addSetting<int>("CaptureMin", 0);
    audioGroup.addSetting<int>("CaptureMax", 255);
    audioGroup.addSetting<int>("PlaybackVolume", 150);
    audioGroup.addSetting<int>("CaptureVolume", 150);
    audioGroup.addSetting<int>("Type", static_cast<const int>(f1x::openauto::common::Enum::AudioOutputType::RTAUDIO));
    audioGroup.addSetting<QString>("PlaybackDevice", "");
    audioGroup.addSetting<QString>("CaptureDevice", "");
    audioGroup.load(m_settings);
    m_configurationGroups.append(audioGroup);

    ConfigurationGroup mediaGroup("Media");
    mediaGroup.addSetting<bool>("AutoPlayback", false);
    mediaGroup.addSetting<bool>("AutoStart", false);
    mediaGroup.load(m_settings);
    m_configurationGroups.append(mediaGroup);

    ConfigurationGroup aaGroup("AndroidAuto");
    aaGroup.addSetting<bool>("Media", true);
    aaGroup.addSetting<bool>("Guidance", true);
    aaGroup.addSetting<bool>("Telephony", false);
    aaGroup.addSetting<int>("FrameRate", aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_60);
    aaGroup.addSetting<int>("Resolution", aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_800x480);
    aaGroup.load(m_settings);
    m_configurationGroups.append(aaGroup);

    ConfigurationGroup bluetoothGroup("Bluetooth");
    bluetoothGroup.addSetting<bool>("Enabled", true);
    bluetoothGroup.addSetting<QString>("AdapterAddress", "");
    bluetoothGroup.addSetting<QString>("PairedDeviceAddress", "");
    bluetoothGroup.load(m_settings);
    m_configurationGroups.append(bluetoothGroup);
    ConfigurationGroup wirelessGroup("Wireless");
    wirelessGroup.addSetting<bool>("Enabled", true);
    wirelessGroup.addSetting<QString>("HotspotSSID", "JourneyOS");
    wirelessGroup.addSetting<QString>("HotspotPassword", generateRandomString(8));
    wirelessGroup.addSetting<QString>("ClientSSID", "");
    wirelessGroup.addSetting<QString>("ClientPassword", "");

    wirelessGroup.addSetting<common::Enum::WirelessType::Value>("Type", f1x::openauto::common::Enum::WirelessType::WIRELESS_HOTSPOT);
    wirelessGroup.load(m_settings);
    m_configurationGroups.append(wirelessGroup);
  }

  void Configuration::save() const {
    // Initialise Settings
    QSettings settings("journey.conf", QSettings::IniFormat);

    for (const auto& group : m_configurationGroups) {
      group.save(settings);
    }
  }

  /// To generate a random string for a wifi password of reqyested length
  /// \param length
  /// \return
  QString Configuration::generateRandomString(size_t length) {
    // Character set including uppercase, lowercase, and digits
    auto charset = QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    QString randomString;

    // Use Qt's random number generator
    for (size_t i = 0; i < length; ++i) {
      int randomIndex = QRandomGenerator::global()->bounded(charset.length());
      randomString.append(charset.at(randomIndex));
    }

    return randomString;
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



