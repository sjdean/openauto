#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <QInputDevice>
#include <QSettings>
#include <QRandomGenerator>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include "f1x/openauto/Common/Enum/WirelessType.hpp"

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcConfig, "journeyos.configuration")

namespace f1x::openauto::autoapp::configuration {

  Configuration::Configuration()
  : m_settings("journey.conf", QSettings::IniFormat)
  {
    m_configurationGroups.clear();

    // Initialise and Configure Default Settings prior to loading...
    ConfigurationGroup carGroup(toQString(ConfigGroup::Car));
    carGroup.addSetting<QString>(toQString(ConfigKey::CarMake), "Unknown");
    carGroup.addSetting<QString>(toQString(ConfigKey::CarModel), "Unknown");
    carGroup.addSetting<int>(toQString(ConfigKey::CarFuelType), aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_UNKNOWN);
    carGroup.addSetting<int>(toQString(ConfigKey::CarEvConnectorType), aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_UNKNOWN);
    carGroup.addSetting<int>(toQString(ConfigKey::CarDriverPosition), aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
    carGroup.addSetting<QString>(toQString(ConfigKey::CarId), generateRandomString(128));
    carGroup.load(m_settings);
    m_configurationGroups.append(carGroup);

    ConfigurationGroup screenGroup(toQString(ConfigGroup::Screen));
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenDayMin), 10);
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenDayMax), 150);
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenNightMin), 10);
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenNightMax), 150);
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenBrightness), 150);
    screenGroup.addSetting<int>(toQString(ConfigKey::ScreenDPI), 160);
    screenGroup.addSetting<bool>(toQString(ConfigKey::ScreenUseClockDayNight), false);
    screenGroup.addSetting<bool>(toQString(ConfigKey::ScreenDarkMode), true);
    screenGroup.load(m_settings);
    m_configurationGroups.append(screenGroup);

    ConfigurationGroup videoGroup(toQString(ConfigGroup::Video));
    videoGroup.addSetting<int>(toQString(ConfigKey::VideoHeight), 0);
    videoGroup.addSetting<int>(toQString(ConfigKey::VideoWidth), 0);
    videoGroup.addSetting<bool>(toQString(ConfigKey::VideoRotate), false);
    videoGroup.load(m_settings);
    m_configurationGroups.append(videoGroup);

    ConfigurationGroup audioGroup(toQString(ConfigGroup::Audio));
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioPlaybackMin), 0);
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioPlaybackMax), 255);
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioCaptureMin), 0);
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioCaptureMax), 255);
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioPlaybackVolume), 150);
    audioGroup.addSetting<int>(toQString(ConfigKey::AudioCaptureVolume), 150);
    audioGroup.addSetting<bool>(toQString(ConfigKey::AudioDebugRecord), true);
    audioGroup.addSetting<QString>(toQString(ConfigKey::AudioPlaybackDevice), "");
    audioGroup.addSetting<QString>(toQString(ConfigKey::AudioCaptureDevice), "");
    audioGroup.load(m_settings);
    m_configurationGroups.append(audioGroup);

    ConfigurationGroup mediaGroup(toQString(ConfigGroup::Media));
    mediaGroup.addSetting<bool>(toQString(ConfigKey::MediaAutoPlayback), false);
    mediaGroup.addSetting<bool>(toQString(ConfigKey::MediaAutoStart), false);
    mediaGroup.load(m_settings);
    m_configurationGroups.append(mediaGroup);

    ConfigurationGroup aaGroup(toQString(ConfigGroup::AndroidAuto));
    aaGroup.addSetting<bool>(toQString(ConfigKey::AndroidAutoMedia), true);
    aaGroup.addSetting<bool>(toQString(ConfigKey::AndroidAutoGuidance), true);
    aaGroup.addSetting<bool>(toQString(ConfigKey::AndroidAutoTelephony), false);
    aaGroup.addSetting<int>(toQString(ConfigKey::AndroidAutoFrameRate), aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_60);
    aaGroup.addSetting<int>(toQString(ConfigKey::AndroidAutoResolution), aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_800x480);
    aaGroup.addSetting<QString>(toQString(ConfigKey::AndroidAutoHomeButtonVisibility), "touchToReveal");
    aaGroup.load(m_settings);
    m_configurationGroups.append(aaGroup);

    ConfigurationGroup bluetoothGroup(toQString(ConfigGroup::Bluetooth));
    bluetoothGroup.addSetting<bool>(toQString(ConfigKey::BluetoothEnabled), true);
    bluetoothGroup.addSetting<QString>(toQString(ConfigKey::BluetoothAdapterAddress), "");
    bluetoothGroup.addSetting<QString>(toQString(ConfigKey::BluetoothPairedDeviceAddress), "");
    bluetoothGroup.addSetting<QString>(toQString(ConfigKey::BluetoothIgnoredDevices), "");
    bluetoothGroup.load(m_settings);
    m_configurationGroups.append(bluetoothGroup);

    ConfigurationGroup systemGroup(toQString(ConfigGroup::System));
    systemGroup.addSetting<bool>(toQString(ConfigKey::SystemHeadUnitMode), true);
    systemGroup.addSetting<bool>(toQString(ConfigKey::SystemDesktopMode), false);
    systemGroup.load(m_settings);
    m_configurationGroups.append(systemGroup);

    ConfigurationGroup wirelessGroup(toQString(ConfigGroup::Wireless));
    wirelessGroup.addSetting<bool>(toQString(ConfigKey::WirelessEnabled), true);
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessHotspotSSID), "JourneyOS");
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessHotspotPassword), generateRandomString(8));
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessClientSSID), "");
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessClientPassword), "");
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessInterface), "");
    wirelessGroup.addSetting<QString>(toQString(ConfigKey::WirelessInterfaceMAC), "");
    wirelessGroup.addSetting<common::Enum::WirelessType::Value>(toQString(ConfigKey::WirelessType), f1x::openauto::common::Enum::WirelessType::WIRELESS_HOTSPOT);
    wirelessGroup.load(m_settings);
    m_configurationGroups.append(wirelessGroup);
  }

  void Configuration::save() const {
    m_settings.sync();
  }

  void Configuration::onSettingChanged(const QString& group, const QString& key, const QVariant& value) {
    m_settings.beginGroup(group);
    m_settings.setValue(key, value);
    m_settings.endGroup();
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
    // Return cached value if we already checked
    if (hasTouchScreenCache_.has_value()) {
      return hasTouchScreenCache_.value();
    }

    // Actual Logic
    const auto devices = QInputDevice::devices();
    int touchCount = 0;
    for (const auto &device : devices) {
      if (device->type() == QInputDevice::DeviceType::TouchScreen) {
        touchCount++;
      }
    }

    // Only log THIS once
    qInfo(lcConfig) << "Touchscreen check: Found" << touchCount << "devices.";

    hasTouchScreenCache_ = (touchCount > 0);
    return hasTouchScreenCache_.value();
  }

}



