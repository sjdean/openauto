#include <QtCore/QObject>
#include <QProcess>
#include <f1x/openauto/autoapp/UI/SettingsView.hpp>
#include <service/control/message/DriverPosition.pb.h>
#include <service/media/sink/message/VideoFrameRateType.pb.h>
#include <service/sensorsource/message/EvConnectorType.pb.h>
#include <service/sensorsource/message/FuelType.pb.h>

#include "f1x/openauto/autoapp/UI/Enum/AudioOutputType.hpp"
#include "f1x/openauto/autoapp/UI/Enum/VideoType.hpp"
#include "f1x/openauto/autoapp/UI/Enum/WirelessType.hpp"

namespace f1x::openauto::autoapp::UI {
  SettingsView::SettingsView(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration,
                             QObject *parent) :
      QObject(parent),
      configuration_(std::move(configuration)),
      m_carId(configuration->getSettingByName<QString>("Car", "Id")),
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
      m_videoType(configuration_->getSettingByName<Enum::VideoType::Value>("Video", "Type")),
      m_videoRotateDisplay(configuration_->getSettingByName<bool>("Video", "Rotate")),

      m_mediaAutoPlayback(configuration_->getSettingByName<bool>("Media", "AutoPlayback")),
      m_mediaAutoStart(configuration_->getSettingByName<bool>("Media", "AutoStart")),

      m_aaChannelMedia(configuration_->getSettingByName<bool>("AndroidAuto", "Media")),
      m_aaChannelGuidance(configuration_->getSettingByName<bool>("AndroidAuto", "Guidance")),
      m_aaChannelTelephony(configuration_->getSettingByName<bool>("AndroidAuto", "Telephony")),
      m_aaFrameRate(configuration_->getSettingByName<aap_protobuf::service::media::sink::message::VideoFrameRateType>("AndroidAuto", "FrameRate")),
      m_aaResolution(configuration_->getSettingByName<aap_protobuf::service::media::sink::message::VideoCodecResolutionType>("AndroidAuto", "Resolution")),

      m_audioVolumePlaybackMin(configuration_->getSettingByName<int>("Audio", "PlaybackMin")),
      m_audioVolumePlaybackMax(configuration_->getSettingByName<int>("Audio", "PlaybackMax")),
      m_audioVolumeCaptureMin(configuration_->getSettingByName<int>("Audio", "CaptureMin")),
      m_audioVolumeCaptureMax(configuration_->getSettingByName<int>("Audio", "CaptureMax")),
      m_audioVolumePlayback(configuration_->getSettingByName<int>("Audio", "PlaybackVolume")),
      m_audioVolumeCapture(configuration_->getSettingByName<int>("Audio", "CaptureVolume")),
      m_audioCaptureDeviceValue(configuration_->getSettingByName<QString>("Audio", "CaptureDevice")),
      m_audioPlaybackDeviceValue(configuration_->getSettingByName<QString>("Audio", "PlaybackDevice")),

      m_audioType(
          static_cast<Enum::AudioOutputType::Value>(configuration_->getSettingByName<int>(
              "Audio", "Type"))),
      m_wirelessClientSSID(configuration_->getSettingByName<QString>("Wireless", "ClientSSID")),
      m_wirelessClientPassword(configuration_->getSettingByName<QString>("Wireless", "ClientPassword")),
      m_wirelessHotspotSSID(configuration_->getSettingByName<QString>("Wireless", "HotspotSSID")),
      m_wirelessHotspotPassword(configuration_->getSettingByName<QString>("Wireless", "HotspotPassword")),

      m_wirelessType(configuration_->getSettingByName<Enum::WirelessType::Value>("Wireless", "Type")),
      m_wirelessEnabled(configuration_->getSettingByName<bool>("Wireless", "Enabled")) {

  }

  QString SettingsView::getCarMake() const {
    return m_carMake;
  }

  QString SettingsView::getCarId() const {
    return QString();
  }

  void SettingsView::setCarId(QString value) {

  }

  aap_protobuf::service::media::sink::message::VideoFrameRateType SettingsView::getAAFrameRate() const {
    return aap_protobuf::service::media::sink::message::VIDEO_FPS_60;
  }

  aap_protobuf::service::media::sink::message::VideoCodecResolutionType SettingsView::getAAResolution() const {
    return aap_protobuf::service::media::sink::message::VIDEO_2160x3840;
  }

  void SettingsView::setAAFrameRate(aap_protobuf::service::media::sink::message::VideoFrameRateType value) {

  }

  void SettingsView::setAAResolution(aap_protobuf::service::media::sink::message::VideoCodecResolutionType value) {

  }

  QString SettingsView::getAudioPlaybackDevice() const {
    return m_audioPlaybackDeviceValue;
  }

  QString SettingsView::getAudioCaptureDevice() const {
    return m_audioCaptureDeviceValue;
  }

  void SettingsView::setAudioPlaybackDevice(QString value) {
    if (m_audioPlaybackDeviceValue != value) {
      configuration_->updateSettingByName<QString>("Car", "Model", value);
      configuration_->save();
      m_audioPlaybackDeviceValue = value;
      emit audioPlaybackDeviceChanged();
    }
  }

  void SettingsView::setAudioCaptureDevice(QString value) {
    if (m_audioCaptureDeviceValue != value) {
      configuration_->updateSettingByName<QString>("Car", "Model", value);
      configuration_->save();
      m_audioCaptureDeviceValue = value;
      emit audioCaptureDeviceChanged();
    }
  }

  QString SettingsView::getHwBluetoothAdapter() {
    return m_hwBluetoothAdapter;
  }

  void SettingsView::setHwBluetoothAdapter(QString value) {
    if (m_hwBluetoothAdapter != value) {
      configuration_->updateSettingByName<QString>("Car", "Model", value);
      configuration_->save();
      m_hwBluetoothAdapter = value;
      emit hwBluetoothAdapterChanged();
    }
  }

  void SettingsView::setCarMake(QString value) {
    fprintf(stderr, "SettingsView::setCarMake %s\n", value.toStdString().c_str());
    if (m_carMake != value) {
      configuration_->updateSettingByName<QString>("Car", "Make", value);
      configuration_->save();
      m_carMake = value;
      emit carMakeChanged();
    }
  }

  QString SettingsView::getCarModel() const {
    return m_carModel;
  }

  void SettingsView::setCarModel(QString value) {
    if (m_carModel != value) {
      configuration_->updateSettingByName<QString>("Car", "Model", value);
      configuration_->save();
      m_carModel = value;
      emit carModelChanged();
    }
  }

  bool SettingsView::getMediaAutoPlayback() const {
    return m_mediaAutoPlayback;
  }

  void SettingsView::setMediaAutoPlayback(bool value) {
    if (m_mediaAutoPlayback != value) {
      configuration_->updateSettingByName<bool>("Media", "AutoPlayback", value);
      configuration_->save();
      m_mediaAutoPlayback = value;
      emit mediaAutoPlaybackChanged();
    }
  }

  bool SettingsView::getMediaAutoStart() const {
    return m_mediaAutoStart;
  }

  void SettingsView::setMediaAutoStart(bool value) {
    if (m_mediaAutoStart != value) {
      configuration_->updateSettingByName<bool>("Media", "AutoStart", value);
      configuration_->save();
      m_mediaAutoStart = value;
      emit mediaAutoStartChanged();
    }
  }

  bool SettingsView::getAAChannelMedia() const {
    return m_aaChannelMedia;
  }

  void SettingsView::setAAChannelMedia(bool value) {
    if (m_aaChannelMedia != value) {
      configuration_->updateSettingByName<bool>("AndroidAuto", "Media", value);
      configuration_->save();
      m_aaChannelMedia = value;
      emit aaChannelMediaChanged();
    }
  }

  bool SettingsView::getAAChannelGuidance() const {
    return m_aaChannelGuidance;
  }

  void SettingsView::setAAChannelGuidance(bool value) {
    if (m_aaChannelGuidance != value) {
      configuration_->updateSettingByName<bool>("AndroidAuto", "Guidance", value);
      configuration_->save();
      m_aaChannelGuidance = value;
      emit aaChannelGuidanceChanged();
    }
  }

  bool SettingsView::getAAChannelTelephony() const {
    return m_aaChannelTelephony;
  }

  void SettingsView::setAAChannelTelephony(bool value) {
    if (m_aaChannelTelephony != value) {
      configuration_->updateSettingByName<bool>("AndroidAuto", "Telephony", value);
      configuration_->save();
      m_aaChannelTelephony = value;
      emit aaChannelTelephonyChanged();
    }
  }

  int SettingsView::getVideoOMXLayer() const {
    return m_videoOMXLayer;
  }

  void SettingsView::setVideoOMXLayer(int value) {
    if (value != m_videoOMXLayer) {
      configuration_->updateSettingByName<int>("Video", "OMXLayer", value);
      configuration_->save();
      m_videoOMXLayer = value;
      emit videoOMXLayerChanged();
    }
  }

  int SettingsView::getVideoMarginHeight() const {
    return m_videoMarginHeight;
  }

  void SettingsView::setVideoMarginHeight(int value) {
    if (value != m_videoMarginHeight) {
      configuration_->updateSettingByName<int>("Video", "Height", value);
      configuration_->save();
      m_videoMarginHeight = value;
      emit videoMarginHeightChanged();
    }
  }

  int SettingsView::getVideoMarginWidth() const {
    return m_videoMarginWidth;
  }

  void SettingsView::setVideoMarginWidth(int value) {
    if (value != m_videoMarginWidth) {
      configuration_->updateSettingByName<int>("Video", "Width", value);
      configuration_->save();
      m_videoMarginWidth = value;
      emit videoMarginWidthChanged();
    }
  }

  int SettingsView::getAudioVolumePlayback() const {
    return m_audioVolumePlayback;
  }

  void SettingsView::setAudioVolumePlayback(int value) {
    if (value != m_audioVolumePlayback) {
      configuration_->updateSettingByName<int>("Audio", "PlaybackVolume", value);
      configuration_->save();
      m_audioVolumePlayback = value;
      emit audioVolumePlaybackChanged();
    }
  }

  int SettingsView::getAudioVolumeCapture() const {
    return m_audioVolumeCapture;
  }

  void SettingsView::setAudioVolumeCapture(int value) {
    if (value != m_audioVolumeCapture) {
      configuration_->updateSettingByName<int>("Car", "CaptureVolume", value);
      configuration_->save();
      m_audioVolumeCapture = value;
      emit audioVolumeCaptureChanged();
    }
  }

  int SettingsView::getScreenBrightnessDayMin() const {
    return m_screenBrightnessDayMin;
  }

  void SettingsView::setScreenBrightnessDayMin(int value) {
    if (value != m_screenBrightnessDayMin) {
      configuration_->updateSettingByName<int>("Screen", "DayMin", value);
      configuration_->save();
      m_screenBrightnessDayMin = value;
      emit screenBrightnessDayMinChanged();
    }
  }

  int SettingsView::getScreenBrightnessDayMax() const {
    return m_screenBrightnessDayMax;
  }

  void SettingsView::setScreenBrightnessDayMax(int value) {
    if (value != m_screenBrightnessDayMax) {
      configuration_->updateSettingByName<int>("Screen", "DayMax", value);
      configuration_->save();
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
      configuration_->updateSettingByName<int>("Screen", "NightMax", value);
      configuration_->save();
      m_screenBrightnessNightMax = value;
      emit screenBrightnessNightMaxChanged();
    }
  }

  bool SettingsView::getVideoRotateDisplay() const {
    return m_videoRotateDisplay;
  }

  void SettingsView::setVideoRotateDisplay(bool value) {
    if (m_videoRotateDisplay != value) {
      configuration_->updateSettingByName<bool>("Video", "Rotate", value);
      configuration_->save();
      m_videoRotateDisplay = value;
      emit videoRotateDisplayChanged();
    }
  }

  int SettingsView::getScreenDPI() const {
    return m_screenDPI;
  }

  void SettingsView::setScreenDPI(int value) {
    if (value != m_screenDPI) {
      configuration_->updateSettingByName<int>("Screen", "DPI", value);
      configuration_->save();
      m_screenDPI = value;
      emit screenDPIChanged();
    }
  }

  void SettingsView::setAudioType(Enum::AudioOutputType::Value value) {
    if (value != m_audioType) {
      configuration_->updateSettingByName<int>("Audio", "Type", static_cast<int>(value));
      configuration_->save();
      m_audioType = value;
      emit audioTypeChanged(value);
    }
  }

  void SettingsView::setAudioVolumeCaptureMax(int value) {
    if (value != m_audioVolumeCaptureMax) {
      configuration_->updateSettingByName<int>("Audio", "CaptureMax", value);
      configuration_->save();
      m_audioVolumeCaptureMax = value;
      emit audioVolumeCaptureMaxChanged();
    }
  }

  void SettingsView::setAudioVolumeCaptureMin(int value) {
    if (value != m_audioVolumeCaptureMin) {
      configuration_->updateSettingByName<int>("Audio", "CaptureMin", value);
      configuration_->save();
      m_audioVolumeCaptureMin = value;
      emit audioVolumeCaptureMinChanged();
    }
  }

  void SettingsView::setAudioVolumePlaybackMax(int value) {
    if (value != m_audioVolumePlaybackMax) {
      configuration_->updateSettingByName<int>("Audio", "PlaybackMax", value);
      configuration_->save();
      m_audioVolumePlaybackMax = value;
      emit audioVolumePlaybackMaxChanged();
    }
  }

  void SettingsView::setAudioVolumePlaybackMin(int value) {
    if (value != m_audioVolumePlaybackMin) {
      configuration_->updateSettingByName<int>("Audio", "PlaybackMin", value);
      configuration_->save();
      m_audioVolumePlaybackMin = value;
      emit audioVolumePlaybackMinChanged();
    }
  }

  Enum::AudioOutputType::Value SettingsView::getAudioType() const {
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

  void SettingsView::setVideoType(Enum::VideoType::Value value) {
    if (value != m_videoType) {
      configuration_->updateSettingByName<Enum::VideoType::Value>("Video", "Type", value);
      configuration_->save();
      m_videoType = value;
      emit videoTypeChanged(value);
    }
  }

  Enum::VideoType::Value SettingsView::getVideoType() const {
    return m_videoType;
  }

  void SettingsView::setScreenBrightness(int value) {
    if (value != m_screenBrightness) {
      configuration_->updateSettingByName<int>("Screen", "Brightness", value);
      configuration_->save();
      m_screenBrightness = value;
      emit screenBrightnessChanged();
    }
  }

  int SettingsView::getScreenBrightness() const {
    return m_screenBrightness;
  }

  void SettingsView::setCarDriverPosition(aap_protobuf::service::control::message::DriverPosition value) {
    if (value != m_carDriverPosition) {
      configuration_->updateSettingByName<aap_protobuf::service::control::message::DriverPosition>("Car", "DriverPosition", value);
      configuration_->save();
      m_carDriverPosition = value;
      emit carDriverPositionChanged();
    }
  }

  void SettingsView::setCarEvConnectorType(aap_protobuf::service::sensorsource::message::EvConnectorType value) {
    if (value != m_carEvConnectorType) {
      configuration_->updateSettingByName<aap_protobuf::service::sensorsource::message::EvConnectorType>("Car", "EvConnectorType", value);
      configuration_->save();
      m_carEvConnectorType = value;
      emit carEvConnectorTypeChanged();
    }
  }

  void SettingsView::setCarFuelType(aap_protobuf::service::sensorsource::message::FuelType value) {
    if (value != m_carFuelType) {
      configuration_->updateSettingByName<aap_protobuf::service::sensorsource::message::FuelType>("Car", "FuelType", value);
      configuration_->save();
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

  void SettingsView::setWirelessClientSSID(QString value) {
    if (m_carMake != value) {
      configuration_->updateSettingByName<QString>("Wireless", "SSID", value);
      configuration_->save();

      m_wirelessClientSSID = value;
      QSettings settings("/etc/wpa_supplicant/wpa_supplicant.conf", QSettings::IniFormat);

      // Assuming you want to update or add a network block
      settings.beginGroup("network");
      settings.setValue("ssid", QString("\"%1\"").arg(m_wirelessClientSSID)); // Quote the SSID since it's typically in quotes
      settings.setValue("id_str", "network_" + m_wirelessClientSSID.toLower().replace(' ', '_')); // A unique identifier
      settings.endGroup();

      // Sync to write changes
      settings.sync();

      // Reconfigure wpa_supplicant to apply changes
      QProcess::execute("sudo", QStringList() << "wpa_cli" << "-i" << "wlan0" << "reconfigure");
      emit wirelessClientSSIDChanged();
    }
  }

  void SettingsView::setWirelessClientPassword(QString value) {
    if (m_carMake != value) {
      configuration_->updateSettingByName<QString>("Wireless", "Password", value);
      configuration_->save();
      m_wirelessClientPassword = value;

      QSettings settings("/etc/wpa_supplicant/wpa_supplicant.conf", QSettings::IniFormat);

      // Assuming you want to update or add a network block
      settings.beginGroup("network");
      settings.setValue("psk", m_wirelessClientPassword);
      settings.endGroup();

      // Sync to write changes
      settings.sync();

      // Reconfigure wpa_supplicant to apply changes
      QProcess::execute("sudo", QStringList() << "wpa_cli" << "-i" << "wlan0" << "reconfigure");

      emit wirelessClientPasswordChanged();
    }
  }

  QString SettingsView::getWirelessClientSSID() {
    return m_wirelessClientSSID;
  }

  QString SettingsView::getWirelessClientPassword() {
    return m_wirelessClientPassword;
  }

  void SettingsView::setWirelessHotspotSSID(QString value) {
    if (m_carMake != value) {
      configuration_->updateSettingByName<QString>("Wireless", "SSID", value);
      configuration_->save();

      m_wirelessHotspotSSID = value;
      QSettings settings("/etc/hostapd/hostapd.conf", QSettings::IniFormat);

      // Update SSID
      settings.setValue("ssid", m_wirelessHotspotSSID);

      // Sync to write changes
      settings.sync();

      // Restart hostapd to apply changes
      QProcess::execute("sudo", QStringList() << "systemctl" << "restart" << "hostapd");

      emit wirelessHotspotSSIDChanged();
    }
  }

  void SettingsView::setWirelessHotspotPassword(QString value) {
    if (m_carMake != value) {
      configuration_->updateSettingByName<QString>("Wireless", "Password", value);
      configuration_->save();
      m_wirelessHotspotPassword = value;
      QSettings settings("/etc/hostapd/hostapd.conf", QSettings::IniFormat);

      // Update wpa_passphrase
      settings.setValue("wpa_passphrase", m_wirelessHotspotPassword);

      // Sync to write changes
      settings.sync();

      // Restart hostapd to apply changes
      QProcess::execute("sudo", QStringList() << "systemctl" << "restart" << "hostapd");

      emit wirelessHotspotPasswordChanged();
    }
  }

  QString SettingsView::getWirelessHotspotSSID() {
    return m_wirelessHotspotSSID;
  }

  QString SettingsView::getWirelessHotspotPassword() {
    return m_wirelessHotspotPassword;
  }

  bool SettingsView::getWirelessEnabled() {
    return m_wirelessEnabled;
  }

  Enum::WirelessType::Value SettingsView::getWirelessType() {
    return m_wirelessType;
  }

  void SettingsView::setWirelessEnabled(bool value) {
    if (value != m_wirelessEnabled) {
      configuration_->updateSettingByName<int>("Wireless", "Enabled", value);
      configuration_->save();
      m_wirelessEnabled = value;
      activateWireless();
      emit wirelessEnabledChanged();
    }
  }

  void SettingsView::activateWireless() {
    if (m_wirelessEnabled) {
      if (m_wirelessType == Enum::WirelessType::Value::WIRELESS_HOTSPOT) {
        QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "hostapd");
        QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "dnsmasq");
        QProcess::execute("sudo", QStringList() << "iptables" << "-F");
        QProcess::execute("sudo", QStringList() << "iptables" << "-X");
        QProcess::execute("sudo", QStringList() << "iptables" << "-t" << "nat" << "-F");
        QProcess::execute("sudo", QStringList() << "ifconfig" << "wlan0" << "down");
        QProcess::execute("sudo", QStringList() << "ifconfig" << "wlan0" << "up");
      } else {
        QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "wpa_supplicant");
        QProcess::execute("sudo", QStringList() << "ifconfig" << "wlan0" << "down");
        QProcess::execute("sudo",
                          QStringList() << "ifconfig" << "wlan0" << "192.168.254.1" << "netmask" << "255.255.255.240"
                                        << "up");
        QProcess::execute("sudo",
                          QStringList() << "iptables" << "-t" << "nat" << "-A" << "POSTROUTING" << "-o" << "eth0"
                                        << "-j" << "MASQUERADE");
        QProcess::execute("sudo",
                          QStringList() << "iptables" << "-A" << "FORWARD" << "-i" << "wlan0" << "-o" << "eth0" << "-j"
                                        << "ACCEPT");
        QProcess::execute("sudo", QStringList() << "systemctl" << "start" << "hostapd");
        QProcess::execute("sudo", QStringList() << "systemctl" << "start" << "dnsmasq");
      }
    } else {
      QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "hostapd");
      QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "dnsmasq");
      QProcess::execute("sudo", QStringList() << "systemctl" << "stop" << "wpa_supplicant");
    }
  }

  void SettingsView::setWirelessType(Enum::WirelessType::Value value) {
    if (value != m_wirelessType) {
      configuration_->updateSettingByName<Enum::WirelessType::Value>("Wireless", "Type", value);
      configuration_->save();
      m_wirelessType = value;
      activateWireless();
      emit wirelessTypeChanged();
    }
  }
}
