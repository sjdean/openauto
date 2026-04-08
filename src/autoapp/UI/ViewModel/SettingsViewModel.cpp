#include <QtCore/QObject>
#include <QProcess>
#include <f1x/openauto/autoapp/UI/ViewModel/SettingsViewModel.hpp>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/WirelessType.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVmSettings, "journeyos.settings")

namespace f1x::openauto::autoapp::UI::ViewModel {
using configuration::ConfigGroup;
using configuration::ConfigKey;
    SettingsViewModel::SettingsViewModel(configuration::IConfiguration::Pointer configuration,
                                         QObject *parent) : QObject(parent),
                                                            configuration_(std::move(configuration)),
                                                            m_carId(configuration_->getSettingByName<QString>(
                                                                ConfigGroup::Car, ConfigKey::CarId)),
                                                            m_carMake(configuration_->getSettingByName<QString>(
                                                                ConfigGroup::Car, ConfigKey::CarMake)),
                                                            m_carModel(configuration_->getSettingByName<QString>(
                                                                ConfigGroup::Car, ConfigKey::CarModel)),
                                                            m_carFuelType(
                                                                static_cast<
                                                                    aap_protobuf::service::sensorsource::message::FuelType>
                                                                (configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Car, ConfigKey::CarFuelType))),
                                                            m_carEvConnectorType(
                                                                static_cast<
                                                                    aap_protobuf::service::sensorsource::message::EvConnectorType>
                                                                (configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Car, ConfigKey::CarEvConnectorType))),
                                                            m_carDriverPosition(
                                                                static_cast<
                                                                    aap_protobuf::service::control::message::DriverPosition>
                                                                (configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Car, ConfigKey::CarDriverPosition))),

                                                            m_screenBrightnessDayMin(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenDayMin)),
                                                            m_screenBrightnessDayMax(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenDayMax)),
                                                            m_screenBrightnessNightMin(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenNightMin)),
                                                            m_screenBrightnessNightMax(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenNightMax)),
                                                            m_screenBrightness(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenBrightness)),
                                                            m_screenDPI(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Screen, ConfigKey::ScreenDPI)),

                                                            m_videoMarginHeight(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Video, ConfigKey::VideoHeight)),
                                                            m_videoMarginWidth(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Video, ConfigKey::VideoWidth)),

                                                            m_videoRotateDisplay(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::Video, ConfigKey::VideoRotate)),

                                                            m_mediaAutoPlayback(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::Media, ConfigKey::MediaAutoPlayback)),
                                                            m_mediaAutoStart(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::Media, ConfigKey::MediaAutoStart)),

                                                            m_aaChannelMedia(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoMedia)),
                                                            m_aaChannelGuidance(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoGuidance)),
                                                            m_aaChannelTelephony(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoTelephony)),
                                                            m_aaFrameRate(
                                                                configuration_->getSettingByName<
                                                                    aap_protobuf::service::media::sink::message::VideoFrameRateType>(
                                                                    ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoFrameRate)),
                                                            m_aaResolution(
                                                                configuration_->getSettingByName<
                                                                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType>(
                                                                    ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoResolution)),

                                                            m_audioVolumePlaybackMin(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioPlaybackMin)),
                                                            m_audioVolumePlaybackMax(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioPlaybackMax)),
                                                            m_audioVolumeCaptureMin(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioCaptureMin)),
                                                            m_audioVolumeCaptureMax(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioCaptureMax)),
                                                            m_audioVolumePlayback(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume)),
                                                            m_audioVolumeCapture(
                                                                configuration_->getSettingByName<int>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioCaptureVolume)),
                                                            m_audioCaptureDeviceValue(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioCaptureDevice)),
                                                            m_audioPlaybackDeviceValue(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice)),

                                                            m_wirelessClientSSID(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessClientSSID)),
                                                            m_wirelessClientPassword(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessClientPassword)),
                                                            m_wirelessHotspotSSID(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessHotspotSSID)),
                                                            m_wirelessHotspotPassword(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessHotspotPassword)),
                                                            m_wirelessHotspotInterface(
                                                                configuration_->getSettingByName<QString>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessInterface)),
                                                            m_wirelessType(
                                                                configuration_->getSettingByName<
                                                                    common::Enum::WirelessType::Value>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessType)),
                                                            m_wirelessEnabled(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::Wireless, ConfigKey::WirelessEnabled)),
#ifdef Q_OS_LINUX
                                                            m_headUnitMode(
                                                                configuration_->getSettingByName<bool>(
                                                                    ConfigGroup::System, ConfigKey::SystemHeadUnitMode))
#else
                                                            m_headUnitMode(false)
#endif
    {
        m_hwBluetoothAdapter = configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress);
        m_androidAutoHomeButtonVisibility = configuration_->getSettingByName<QString>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoHomeButtonVisibility);
        m_uiDarkMode = configuration_->getSettingByName<bool>(ConfigGroup::Screen, ConfigKey::ScreenDarkMode);
    }

    QString SettingsViewModel::getCarMake() const {
        return m_carMake;
    }

    QString SettingsViewModel::getCarId() const {
        return QString();
    }

    void SettingsViewModel::setCarId(QString value) {
        // TODO: Populate
    }

    aap_protobuf::service::media::sink::message::VideoFrameRateType SettingsViewModel::getAAFrameRate() const {
        return aap_protobuf::service::media::sink::message::VIDEO_FPS_60;
    }

    aap_protobuf::service::media::sink::message::VideoCodecResolutionType SettingsViewModel::getAAResolution() const {
        return aap_protobuf::service::media::sink::message::VIDEO_2160x3840;
    }

    void SettingsViewModel::setAAFrameRate(aap_protobuf::service::media::sink::message::VideoFrameRateType value) {
        // TODO: Populate
    }

    void SettingsViewModel::setAAResolution(
        aap_protobuf::service::media::sink::message::VideoCodecResolutionType value) {
        // TODO: Populate
    }

    QString SettingsViewModel::getAudioPlaybackDevice() const {
        return m_audioPlaybackDeviceValue;
    }

    QString SettingsViewModel::getAudioCaptureDevice() const {
        return m_audioCaptureDeviceValue;
    }

    void SettingsViewModel::setAudioPlaybackDevice(QString value) {
        if (m_audioPlaybackDeviceValue != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice, value);
            configuration_->save();
            m_audioPlaybackDeviceValue = value;
            emit audioPlaybackDeviceChanged();
        }
    }

    void SettingsViewModel::setAudioCaptureDevice(QString value) {
        if (m_audioCaptureDeviceValue != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice, value);
            configuration_->save();
            m_audioCaptureDeviceValue = value;
            emit audioCaptureDeviceChanged();
        }
    }

    QString SettingsViewModel::getHwBluetoothAdapter() {
        return m_hwBluetoothAdapter;
    }

    void SettingsViewModel::setHwBluetoothAdapter(QString value) {
        if (m_hwBluetoothAdapter != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress, value);
            configuration_->save();
            m_hwBluetoothAdapter = value;
            emit hwBluetoothAdapterChanged();
        }
    }

    void SettingsViewModel::setCarMake(QString value) {
        if (m_carMake != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Car, ConfigKey::CarMake, value);
            configuration_->save();
            m_carMake = value;
            emit carMakeChanged();
        }
    }

    QString SettingsViewModel::getCarModel() const {
        return m_carModel;
    }

    void SettingsViewModel::setCarModel(QString value) {
        if (m_carModel != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Car, ConfigKey::CarModel, value);
            configuration_->save();
            m_carModel = value;
            emit carModelChanged();
        }
    }

    bool SettingsViewModel::getMediaAutoPlayback() const {
        return m_mediaAutoPlayback;
    }

    void SettingsViewModel::setMediaAutoPlayback(bool value) {
        if (m_mediaAutoPlayback != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::Media, ConfigKey::MediaAutoPlayback, value);
            configuration_->save();
            m_mediaAutoPlayback = value;
            emit mediaAutoPlaybackChanged();
        }
    }

    bool SettingsViewModel::getMediaAutoStart() const {
        return m_mediaAutoStart;
    }

    void SettingsViewModel::setMediaAutoStart(bool value) {
        if (m_mediaAutoStart != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::Media, ConfigKey::MediaAutoStart, value);
            configuration_->save();
            m_mediaAutoStart = value;
            emit mediaAutoStartChanged();
        }
    }

    bool SettingsViewModel::getAAChannelMedia() const {
        return m_aaChannelMedia;
    }

    void SettingsViewModel::setAAChannelMedia(bool value) {
        if (m_aaChannelMedia != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoMedia, value);
            configuration_->save();
            m_aaChannelMedia = value;
            emit aaChannelMediaChanged();
        }
    }

    bool SettingsViewModel::getAAChannelGuidance() const {
        return m_aaChannelGuidance;
    }

    void SettingsViewModel::setAAChannelGuidance(bool value) {
        if (m_aaChannelGuidance != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoGuidance, value);
            configuration_->save();
            m_aaChannelGuidance = value;
            emit aaChannelGuidanceChanged();
        }
    }

    bool SettingsViewModel::getAAChannelTelephony() const {
        return m_aaChannelTelephony;
    }

    void SettingsViewModel::setAAChannelTelephony(bool value) {
        if (m_aaChannelTelephony != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoTelephony, value);
            configuration_->save();
            m_aaChannelTelephony = value;
            emit aaChannelTelephonyChanged();
        }
    }

    int SettingsViewModel::getVideoMarginHeight() const {
        return m_videoMarginHeight;
    }

    void SettingsViewModel::setVideoMarginHeight(int value) {
        if (value != m_videoMarginHeight) {
            configuration_->updateSettingByName<int>(ConfigGroup::Video, ConfigKey::VideoHeight, value);
            configuration_->save();
            m_videoMarginHeight = value;
            emit videoMarginHeightChanged();
        }
    }

    int SettingsViewModel::getVideoMarginWidth() const {
        return m_videoMarginWidth;
    }

    void SettingsViewModel::setVideoMarginWidth(int value) {
        if (value != m_videoMarginWidth) {
            configuration_->updateSettingByName<int>(ConfigGroup::Video, ConfigKey::VideoWidth, value);
            configuration_->save();
            m_videoMarginWidth = value;
            emit videoMarginWidthChanged();
        }
    }

    int SettingsViewModel::getAudioVolumePlayback() const {
        return m_audioVolumePlayback;
    }

    void SettingsViewModel::setAudioVolumePlayback(int value) {
        if (value != m_audioVolumePlayback) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume, value);
            configuration_->save();
            m_audioVolumePlayback = value;
            emit audioVolumePlaybackChanged();
        }
    }

    int SettingsViewModel::getAudioVolumeCapture() const {
        return m_audioVolumeCapture;
    }

    void SettingsViewModel::setAudioVolumeCapture(int value) {
        if (value != m_audioVolumeCapture) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureVolume, value);
            configuration_->save();
            m_audioVolumeCapture = value;
            emit audioVolumeCaptureChanged();
        }
    }

    int SettingsViewModel::getScreenBrightnessDayMin() const {
        return m_screenBrightnessDayMin;
    }

    void SettingsViewModel::setScreenBrightnessDayMin(int value) {
        if (value != m_screenBrightnessDayMin) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenDayMin, value);
            configuration_->save();
            m_screenBrightnessDayMin = value;
            emit screenBrightnessDayMinChanged();
        }
    }

    int SettingsViewModel::getScreenBrightnessDayMax() const {
        return m_screenBrightnessDayMax;
    }

    void SettingsViewModel::setScreenBrightnessDayMax(int value) {
        if (value != m_screenBrightnessDayMax) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenDayMax, value);
            configuration_->save();
            m_screenBrightnessDayMax = value;
            emit screenBrightnessDayMaxChanged();
        }
    }

    int SettingsViewModel::getScreenBrightnessNightMin() const {
        return m_screenBrightnessNightMin;
    }

    void SettingsViewModel::setScreenBrightnessNightMin(int value) {
        if (value != m_screenBrightnessNightMin) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenNightMin, value);
            configuration_->save();
            m_screenBrightnessNightMin = value;
            emit screenBrightnessNightMinChanged();
        }
    }

    int SettingsViewModel::getScreenBrightnessNightMax() const {
        return m_screenBrightnessNightMax;
    }

    void SettingsViewModel::setScreenBrightnessNightMax(int value) {
        if (value != m_screenBrightnessNightMax) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenNightMax, value);
            configuration_->save();
            m_screenBrightnessNightMax = value;
            emit screenBrightnessNightMaxChanged();
        }
    }

    bool SettingsViewModel::getVideoRotateDisplay() const {
        return m_videoRotateDisplay;
    }

    void SettingsViewModel::setVideoRotateDisplay(bool value) {
        if (m_videoRotateDisplay != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::Video, ConfigKey::VideoRotate, value);
            configuration_->save();
            m_videoRotateDisplay = value;
            emit videoRotateDisplayChanged();
        }
    }

    int SettingsViewModel::getScreenDPI() const {
        return m_screenDPI;
    }

    void SettingsViewModel::setScreenDPI(int value) {
        if (value != m_screenDPI) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenDPI, value);
            configuration_->save();
            m_screenDPI = value;
            emit screenDPIChanged();
        }
    }

    void SettingsViewModel::setAudioVolumeCaptureMax(int value) {
        if (value != m_audioVolumeCaptureMax) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMax, value);
            configuration_->save();
            m_audioVolumeCaptureMax = value;
            emit audioVolumeCaptureMaxChanged();
        }
    }

    void SettingsViewModel::setAudioVolumeCaptureMin(int value) {
        if (value != m_audioVolumeCaptureMin) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMin, value);
            configuration_->save();
            m_audioVolumeCaptureMin = value;
            emit audioVolumeCaptureMinChanged();
        }
    }

    void SettingsViewModel::setAudioVolumePlaybackMax(int value) {
        if (value != m_audioVolumePlaybackMax) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMax, value);
            configuration_->save();
            m_audioVolumePlaybackMax = value;
            emit audioVolumePlaybackMaxChanged();
        }
    }

    void SettingsViewModel::setAudioVolumePlaybackMin(int value) {
        if (value != m_audioVolumePlaybackMin) {
            configuration_->updateSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMin, value);
            configuration_->save();
            m_audioVolumePlaybackMin = value;
            emit audioVolumePlaybackMinChanged();
        }
    }

    int SettingsViewModel::getAudioVolumeCaptureMax() const {
        return m_audioVolumeCaptureMax;
    }

    int SettingsViewModel::getAudioVolumeCaptureMin() const {
        return m_audioVolumeCaptureMin;
    }

    int SettingsViewModel::getAudioVolumePlaybackMax() const {
        return m_audioVolumePlaybackMax;
    }

    int SettingsViewModel::getAudioVolumePlaybackMin() const {
        return m_audioVolumePlaybackMin;
    }

    void SettingsViewModel::setScreenBrightness(int value) {
        if (value != m_screenBrightness) {
            configuration_->updateSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenBrightness, value);
            configuration_->save();
            m_screenBrightness = value;
            emit screenBrightnessChanged();
        }
    }

    int SettingsViewModel::getScreenBrightness() const {
        return m_screenBrightness;
    }

    void SettingsViewModel::setCarDriverPosition(aap_protobuf::service::control::message::DriverPosition value) {
        if (value != m_carDriverPosition) {
            configuration_->updateSettingByName<aap_protobuf::service::control::message::DriverPosition>(
                ConfigGroup::Car, ConfigKey::CarDriverPosition, value);
            configuration_->save();
            m_carDriverPosition = value;
            emit carDriverPositionChanged();
        }
    }

    void SettingsViewModel::setCarEvConnectorType(aap_protobuf::service::sensorsource::message::EvConnectorType value) {
        if (value != m_carEvConnectorType) {
            configuration_->updateSettingByName<aap_protobuf::service::sensorsource::message::EvConnectorType>(
                ConfigGroup::Car, ConfigKey::CarEvConnectorType, value);
            configuration_->save();
            m_carEvConnectorType = value;
            emit carEvConnectorTypeChanged();
        }
    }

    void SettingsViewModel::setCarFuelType(aap_protobuf::service::sensorsource::message::FuelType value) {
        if (value != m_carFuelType) {
            configuration_->updateSettingByName<aap_protobuf::service::sensorsource::message::FuelType>(
                ConfigGroup::Car, ConfigKey::CarFuelType, value);
            configuration_->save();
            m_carFuelType = value;
            emit carFuelTypeChanged();
        }
    }

    aap_protobuf::service::control::message::DriverPosition SettingsViewModel::getCarDriverPosition() const {
        return m_carDriverPosition;
    }

    aap_protobuf::service::sensorsource::message::EvConnectorType SettingsViewModel::getCarEvConnectorType() const {
        return m_carEvConnectorType;
    }

    aap_protobuf::service::sensorsource::message::FuelType SettingsViewModel::getCarFuelType() const {
        return m_carFuelType;
    }

    void SettingsViewModel::setWirelessClientSSID(QString value) {
        if (m_wirelessClientSSID != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessClientSSID, value);
            configuration_->save();

            m_wirelessClientSSID = value;
#ifdef Q_OS_LINUX
            // TODO: Make Agnostic - move to Handler for Linux, NetworkManager specifically.
            QSettings settings("/etc/wpa_supplicant/wpa_supplicant.conf", QSettings::IniFormat);

            // Assuming you want to update or add a network block
            settings.beginGroup("network");
            settings.setValue("ssid", QString("\"%1\"").arg(m_wirelessClientSSID));
            // Quote the SSID since it's typically in quotes
            settings.setValue("id_str", "network_" + m_wirelessClientSSID.toLower().replace(' ', '_'));
            // A unique identifier
            settings.endGroup();

            // Sync to write changes
            settings.sync();

            // Reconfigure wpa_supplicant to apply changes
            QProcess::execute("sudo", QStringList() << "wpa_cli" << "-i" << "wlan0" << "reconfigure");
            emit wirelessClientSSIDChanged();
#else
            qDebug(lcVmSettings) << "desktop mode: skipping wpa_cli for SSID change";
#endif
        }
    }

    void SettingsViewModel::setWirelessClientPassword(QString value) {
        if (m_wirelessClientPassword != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessClientPassword, value);
            configuration_->save();
            m_wirelessClientPassword = value;
#ifdef Q_OS_LINUX
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
#else
            qDebug(lcVmSettings) << "desktop mode: skipping wpa_cli for password change";
#endif
        }
    }

    QString SettingsViewModel::getWirelessClientSSID() {
        return m_wirelessClientSSID;
    }

    QString SettingsViewModel::getWirelessClientPassword() {
        return m_wirelessClientPassword;
    }

    void SettingsViewModel::setWirelessHotspotSSID(QString value) {
        if (m_wirelessHotspotSSID != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessHotspotSSID, value);
            configuration_->save();

            m_wirelessHotspotSSID = value;

#ifdef Q_OS_LINUX
            QSettings settings("/etc/hostapd/hostapd.conf", QSettings::IniFormat);

            // Update SSID
            settings.setValue("ssid", m_wirelessHotspotSSID);

            // Sync to write changes
            settings.sync();

            // Restart hostapd to apply changes
            QProcess::execute("sudo", QStringList() << "systemctl" << "restart" << "hostapd");

            emit wirelessHotspotSSIDChanged();
#else
            qDebug(lcVmSettings) << "desktop mode: skipping hostapd restart for SSID change";
#endif
        }
    }

    void SettingsViewModel::setWirelessHotspotPassword(QString value) {
        if (m_wirelessHotspotPassword != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessHotspotPassword, value);
            configuration_->save();
            m_wirelessHotspotPassword = value;

#ifdef Q_OS_LINUX
            QSettings settings("/etc/hostapd/hostapd.conf", QSettings::IniFormat);

            // Update wpa_passphrase
            settings.setValue("wpa_passphrase", m_wirelessHotspotPassword);

            // Sync to write changes
            settings.sync();

            // Restart hostapd to apply changes
            QProcess::execute("sudo", QStringList() << "systemctl" << "restart" << "hostapd");

            emit wirelessHotspotPasswordChanged();
#else
            qDebug(lcVmSettings) << "desktop mode: skipping hostapd restart for SSID change";
#endif
        }
    }

    QString SettingsViewModel::getWirelessHotspotSSID() {
        return m_wirelessHotspotSSID;
    }

    QString SettingsViewModel::getWirelessHotspotPassword() {
        return m_wirelessHotspotPassword;
    }

    bool SettingsViewModel::getWirelessEnabled() {
        return m_wirelessEnabled;
    }

    common::Enum::WirelessType::Value SettingsViewModel::getWirelessType() {
        return m_wirelessType;
    }

    void SettingsViewModel::setWirelessHotspotInterface(QString value) {
        if (m_wirelessHotspotInterface != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessInterface, value);
            configuration_->save();
            m_wirelessHotspotInterface = value;

#ifdef Q_OS_LINUX
            QSettings settings("/etc/hostapd/hostapd.conf", QSettings::IniFormat);

            // Update the wireless interface used by hostapd
            settings.setValue("interface", m_wirelessHotspotInterface);

            // Sync to write changes
            settings.sync();

            // Restart hostapd to apply changes
            QProcess::execute("sudo", QStringList() << "systemctl" << "restart" << "hostapd");

            emit wirelessHotspotInterfaceChanged();
#else
            qDebug(lcVmSettings) << "desktop mode: skipping hostapd restart for interface change";
#endif
        }
    }

    QString SettingsViewModel::getWirelessHotspotInterface() {
        return m_wirelessHotspotInterface;
    }

    void SettingsViewModel::setWirelessEnabled(bool value) {
        if (value != m_wirelessEnabled) {
            configuration_->updateSettingByName<bool>(ConfigGroup::Wireless, ConfigKey::WirelessEnabled, value);
            configuration_->save();
            m_wirelessEnabled = value;

            emit wirelessEnabledChanged();
        }
    }

    bool SettingsViewModel::isHeadUnitMode() const {
        return m_headUnitMode;
    }

    void SettingsViewModel::setHeadUnitMode(bool value) {
#ifdef Q_OS_LINUX
        if (m_headUnitMode != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::System, ConfigKey::SystemHeadUnitMode, value);
            configuration_->save();
            m_headUnitMode = value;
            emit headUnitModeChanged();
        }
#else
        Q_UNUSED(value) // Head Unit Mode is always off on non-Linux platforms
#endif
    }

    void SettingsViewModel::setWirelessType(common::Enum::WirelessType::Value value) {
        if (value != m_wirelessType) {
            configuration_->updateSettingByName<common::Enum::WirelessType::Value>(ConfigGroup::Wireless, ConfigKey::WirelessType, value);
            configuration_->save();
            m_wirelessType = value;

            emit wirelessTypeChanged();
        }
    }

    QString SettingsViewModel::getAndroidAutoHomeButtonVisibility() const {
        return m_androidAutoHomeButtonVisibility;
    }

    void SettingsViewModel::setAndroidAutoHomeButtonVisibility(QString value) {
        if (m_androidAutoHomeButtonVisibility != value) {
            configuration_->updateSettingByName<QString>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoHomeButtonVisibility, value);
            configuration_->save();
            m_androidAutoHomeButtonVisibility = value;
            emit androidAutoHomeButtonVisibilityChanged();
        }
    }

    bool SettingsViewModel::isUiDarkMode() const {
        return m_uiDarkMode;
    }

    void SettingsViewModel::setUiDarkMode(bool value) {
        if (m_uiDarkMode != value) {
            configuration_->updateSettingByName<bool>(ConfigGroup::Screen, ConfigKey::ScreenDarkMode, value);
            configuration_->save();
            m_uiDarkMode = value;
            emit uiDarkModeChanged();
        }
    }
}
