#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include <algorithm>
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

namespace f1x::openauto::autoapp::UI::Monitor  {
  VolumeHandler::VolumeHandler(configuration::IConfiguration::Pointer configuration, std::shared_ptr<IAudioHandler> audioHandler) :
  configuration_(std::move(configuration)),
  m_audioHandler(std::move(audioHandler)) {
    m_sinkMin   = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    m_sinkMax   = configuration_->getSettingByName<int>("Audio", "PlaybackMax");
    m_sourceMin = configuration_->getSettingByName<int>("Audio", "CaptureMin");
    m_sourceMax = configuration_->getSettingByName<int>("Audio", "CaptureMax");

    // Clamp stored value to current [min,max] range and apply to hardware.
    // If no device is configured, resolve from the audio system and persist so the
    // UI shows a meaningful value on subsequent boots.
    bool settingsChanged = false;

    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    if (device.isEmpty()) {
        device = m_audioHandler->getDefaultSink();
        if (!device.isEmpty()) {
            configuration_->updateSettingByName<QString>("Audio", "PlaybackDevice", device);
            settingsChanged = true;
            qCInfo(lcVolume) << "PlaybackDevice auto-detected sink=" << device;
        }
    }
    const int volume = std::clamp(
        configuration_->getSettingByName<int>("Audio", "PlaybackVolume"), m_sinkMin, m_sinkMax);
    m_audioHandler->setSinkVolume(device, volume);
    m_volumeSink = volume;

    QString captureDevice = configuration_->getSettingByName<QString>("Audio", "CaptureDevice");
    if (captureDevice.isEmpty()) {
        captureDevice = m_audioHandler->getDefaultSource();
        if (!captureDevice.isEmpty()) {
            configuration_->updateSettingByName<QString>("Audio", "CaptureDevice", captureDevice);
            settingsChanged = true;
            qCInfo(lcVolume) << "CaptureDevice auto-detected source=" << captureDevice;
        }
    }
    const int captureVolume = std::clamp(
        configuration_->getSettingByName<int>("Audio", "CaptureVolume"), m_sourceMin, m_sourceMax);
    m_audioHandler->setSourceVolume(captureDevice, captureVolume);
    m_volumeSource = captureVolume;

    if (settingsChanged)
        configuration_->save();
  }

  void VolumeHandler::setVolumeSink(const int volume) {
    const int clamped = std::clamp(volume, m_sinkMin, m_sinkMax);
    const QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    m_audioHandler->setSinkVolume(device, clamped);
    configuration_->updateSettingByName("Audio", "PlaybackVolume", clamped);
    m_volumeSink = clamped;
    emit volumeSinkChanged();
  }

  void VolumeHandler::setVolumeSource(const int volume) {
    const int clamped = std::clamp(volume, m_sourceMin, m_sourceMax);
    const QString device = configuration_->getSettingByName<QString>("Audio", "CaptureDevice");
    m_audioHandler->setSourceVolume(device, clamped);
    configuration_->updateSettingByName("Audio", "CaptureVolume", clamped);
    m_volumeSource = clamped;
    emit volumeSourceChanged();
  }

  void VolumeHandler::setVolumeSinkMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    m_audioHandler->setSinkMute(device, mute);
    m_volumeSinkMute = mute;
    emit volumeSinkMuteChanged();
  }

  void VolumeHandler::setVolumeSourceMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>("Audio", "CaptureDevice");
    m_audioHandler->setSourceMute(device, mute);
    m_volumeSourceMute = mute;
    emit volumeSourceMuteChanged();
  }

  int VolumeHandler::getVolumeSinkMin() const { return m_sinkMin; }
  int VolumeHandler::getVolumeSinkMax() const { return m_sinkMax; }
  int VolumeHandler::getVolumeSourceMin() const { return m_sourceMin; }
  int VolumeHandler::getVolumeSourceMax() const { return m_sourceMax; }

  bool VolumeHandler::getVolumeSourceMute() const {
    return m_volumeSourceMute;
  }

  bool VolumeHandler::getVolumeSinkMute() const {
    return m_volumeSinkMute;
  }

  int VolumeHandler::getVolumeSource() const {
    return m_volumeSource;
  }

  int VolumeHandler::getVolumeSink() const {
    return m_volumeSink;
  }

  void VolumeHandler::saveSettings() const {
    configuration_->save();
  }
}
