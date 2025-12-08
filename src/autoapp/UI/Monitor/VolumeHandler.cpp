#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

namespace f1x::openauto::autoapp::UI::Monitor  {
  VolumeHandler::VolumeHandler(configuration::IConfiguration::Pointer configuration, std::shared_ptr<IAudioHandler> audioHandler) :
  configuration_(std::move(configuration)),
  m_audioHandler(std::move(audioHandler)) {

  }

  void VolumeHandler::setVolumeSink(const int volume) {

    const int min = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    const int max = configuration_->getSettingByName<int>("Audio", "PlaybackMax");

    const int calculatedVolume = calculateVolume(min, max, volume);

    m_audioHandler->setSinkVolume(calculatedVolume);
    configuration_->updateSettingByName("Audio", "PlaybackVolume", volume);

    m_volumeSink = calculatedVolume;
  }

  void VolumeHandler::setVolumeSource(const int volume) {

    const int min = configuration_->getSettingByName<int>("Audio", "CaptureMin");
    const int max = configuration_->getSettingByName<int>("Audio", "CaptureMax");

    const int calculatedVolume = calculateVolume(min, max, volume);

    m_audioHandler->setSourceVolume(calculatedVolume);
    configuration_->updateSettingByName("Audio", "CaptureVolume", volume);

    m_volumeSource = calculatedVolume;
  }

  void VolumeHandler::setVolumeSinkMute(const bool mute) {
    m_audioHandler->setSinkMute(mute);
    m_volumeSinkMute = mute;
  }

  void VolumeHandler::setVolumeSourceMute(const bool mute) {
    m_audioHandler->setSourceMute(mute);
    m_volumeSourceMute = mute;
  }

  int VolumeHandler::calculateVolume(const int min, const int max, const int target) {
    const auto calculatedVolume = min + (((max - min) * target) / 255);
    return calculatedVolume;
  }

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
}
