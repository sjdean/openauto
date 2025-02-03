#include <f1x/openauto/autoapp/UI/VolumeHandler.hpp>
#include <utility>

namespace f1x::openauto::autoapp::UI {
  VolumeHandler::VolumeHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, PulseAudioHandler pulseAudioHandler) :
  configuration_(std::move(configuration)),
  m_pulseAudioHandler(pulseAudioHandler) {

  }

  void VolumeHandler::setVolumeSink(int volume) {

    int min = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    int max = configuration_->getSettingByName<int>("Audio", "PlaybackMax");

    int calculatedVolume = calculateVolume(min, max, volume);

    m_pulseAudioHandler.setSinkVolume(calculatedVolume);
    configuration_->updateSettingByName("Audio", "PlaybackVolume", volume);

    m_volumeSink = calculatedVolume;
  }

  void VolumeHandler::setVolumeSource(int volume) {

    int min = configuration_->getSettingByName<int>("Audio", "CaptureMin");
    int max = configuration_->getSettingByName<int>("Audio", "CaptureMax");

    int calculatedVolume = calculateVolume(min, max, volume);

    m_pulseAudioHandler.setSourceVolume(calculatedVolume);
    configuration_->updateSettingByName("Audio", "CaptureVolume", volume);

    m_volumeSource = calculatedVolume;
  }

  void VolumeHandler::setVolumeSinkMute(bool mute) {
    m_pulseAudioHandler.setSinkMute(mute);
    m_volumeSinkMute = mute;
  }

  void VolumeHandler::setVolumeSourceMute(bool mute) {
    m_pulseAudioHandler.setSourceMute(mute);
    m_volumeSourceMute = mute;
  }

  int VolumeHandler::calculateVolume(int min, int max, int target) {
    auto calculatedVolume = min + (((max - min) * target) / 255);
    return calculatedVolume;
  }

  bool VolumeHandler::getVolumeSourceMute() {
    return m_volumeSourceMute;
  }

  bool VolumeHandler::getVolumeSinkMute() {
    return m_volumeSinkMute;
  }

  int VolumeHandler::getVolumeSource() {
    return m_volumeSource;
  }

  int VolumeHandler::getVolumeSink() {
    return m_volumeSink;
  }
}