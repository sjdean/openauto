#include <f1x/openauto/autoapp/UI/VolumeHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  VolumeHandler::VolumeHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, PulseAudioHandler pulseAudioHandler) :
  configuration_(configuration),
  m_pulseAudioHandler(pulseAudioHandler) {

  }

  void VolumeHandler::setVolumeSink(int volume) {

    int min = configuration_->getSettingByName<int>("Audio", "audioPlaybackVolumeMin");
    int max = configuration_->getSettingByName<int>("Audio", "audioPlaybackVolumeMax");

    int calculatedVolume = calculateVolume(min, max, volume);

    m_pulseAudioHandler.setSinkVolume(calculatedVolume);
    m_volumeSink = calculatedVolume;
  }

  void VolumeHandler::setVolumeSource(int volume) {

    int min = configuration_->getSettingByName<int>("Audio", "audioCaptureVolumeMin");
    int max = configuration_->getSettingByName<int>("Audio", "audioCaptureVolumeMax");

    int calculatedVolume = calculateVolume(min, max, volume);

    m_pulseAudioHandler.setSourceVolume(calculatedVolume);

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