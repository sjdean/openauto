#include "f1x/openauto/autoapp/UI/ViewModel/VolumeViewModel.hpp"
#include <algorithm>
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

namespace f1x::openauto::autoapp::UI::ViewModel {
using configuration::ConfigGroup;
using configuration::ConfigKey;
  VolumeViewModel::VolumeViewModel(configuration::IConfiguration::Pointer configuration, std::shared_ptr<IAudioHandler> audioHandler) :
  configuration_(std::move(configuration)),
  m_audioHandler(std::move(audioHandler)) {
    m_sinkMin   = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMin);
    m_sinkMax   = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMax);
    m_sourceMin = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMin);
    m_sourceMax = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMax);

    // Clamp stored value to current [min,max] range and apply to hardware.
    // If no device is configured, resolve from the audio system and persist so the
    // UI shows a meaningful value on subsequent boots.
    bool settingsChanged = false;

    QString device = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice);
    if (device.isEmpty()) {
        device = m_audioHandler->getDefaultSink();
        if (!device.isEmpty()) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice, device);
            settingsChanged = true;
            qInfo(lcVolume) << "PlaybackDevice auto-detected sink=" << device;
        }
    }
    const int volume = std::clamp(
        configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume), m_sinkMin, m_sinkMax);
    m_audioHandler->setSinkVolume(device, volume);
    m_volumeSink = volume;

    QString captureDevice = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice);
    if (captureDevice.isEmpty()) {
        captureDevice = m_audioHandler->getDefaultSource();
        if (!captureDevice.isEmpty()) {
            configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice, captureDevice);
            settingsChanged = true;
            qInfo(lcVolume) << "CaptureDevice auto-detected source=" << captureDevice;
        }
    }
    const int captureVolume = std::clamp(
        configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureVolume), m_sourceMin, m_sourceMax);
    m_audioHandler->setSourceVolume(captureDevice, captureVolume);
    m_volumeSource = captureVolume;

    if (settingsChanged)
        configuration_->save();
  }

  void VolumeViewModel::setVolumeSink(const int volume) {
    const int clamped = std::clamp(volume, m_sinkMin, m_sinkMax);
    const QString device = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice);
    m_audioHandler->setSinkVolume(device, clamped);
    configuration_->updateSettingByName(ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume, clamped);
    m_volumeSink = clamped;
    emit volumeSinkChanged();
  }

  void VolumeViewModel::setVolumeSource(const int volume) {
    const int clamped = std::clamp(volume, m_sourceMin, m_sourceMax);
    const QString device = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice);
    m_audioHandler->setSourceVolume(device, clamped);
    configuration_->updateSettingByName(ConfigGroup::Audio, ConfigKey::AudioCaptureVolume, clamped);
    m_volumeSource = clamped;
    emit volumeSourceChanged();
  }

  void VolumeViewModel::setVolumeSinkMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice);
    m_audioHandler->setSinkMute(device, mute);
    m_volumeSinkMute = mute;
    emit volumeSinkMuteChanged();
  }

  void VolumeViewModel::setVolumeSourceMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice);
    m_audioHandler->setSourceMute(device, mute);
    m_volumeSourceMute = mute;
    emit volumeSourceMuteChanged();
  }

  int VolumeViewModel::getVolumeSinkMin() const { return m_sinkMin; }
  int VolumeViewModel::getVolumeSinkMax() const { return m_sinkMax; }
  int VolumeViewModel::getVolumeSourceMin() const { return m_sourceMin; }
  int VolumeViewModel::getVolumeSourceMax() const { return m_sourceMax; }

  bool VolumeViewModel::getVolumeSourceMute() const {
    return m_volumeSourceMute;
  }

  bool VolumeViewModel::getVolumeSinkMute() const {
    return m_volumeSinkMute;
  }

  int VolumeViewModel::getVolumeSource() const {
    return m_volumeSource;
  }

  int VolumeViewModel::getVolumeSink() const {
    return m_volumeSink;
  }

  void VolumeViewModel::saveSettings() const {
    configuration_->save();
  }
}
