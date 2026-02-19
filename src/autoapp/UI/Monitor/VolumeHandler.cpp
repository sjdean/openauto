#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

namespace f1x::openauto::autoapp::UI::Monitor  {
  VolumeHandler::VolumeHandler(configuration::IConfiguration::Pointer configuration, std::shared_ptr<IAudioHandler> audioHandler) :
  configuration_(std::move(configuration)),
  m_audioHandler(std::move(audioHandler)) {
    const int min = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    const int max = configuration_->getSettingByName<int>("Audio", "PlaybackMax");
    const int volume = configuration_->getSettingByName<int>("Audio", "PlaybackVolume");
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");

    const int calculatedVolume = calculateVolume(min, max, volume);

    m_audioHandler->setSinkVolume(device, calculatedVolume);
    m_volumeSink = volume;
  }

  void VolumeHandler::setVolumeSink(const int volume) {
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    const int min = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    const int max = configuration_->getSettingByName<int>("Audio", "PlaybackMax");

    const int calculatedVolume = calculateVolume(min, max, volume);

    m_audioHandler->setSinkVolume(device, calculatedVolume);
    configuration_->updateSettingByName("Audio", "PlaybackVolume", volume);
    m_volumeSink = volume;
    emit volumeSinkChanged();
  }

  void VolumeHandler::setVolumeSource(const int volume) {
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    const int min = configuration_->getSettingByName<int>("Audio", "CaptureMin");
    const int max = configuration_->getSettingByName<int>("Audio", "CaptureMax");

    const int calculatedVolume = calculateVolume(min, max, volume);

    m_audioHandler->setSourceVolume(device, calculatedVolume);
    configuration_->updateSettingByName("Audio", "CaptureVolume", volume);
    m_volumeSource = volume;
    emit volumeSourceChanged();
  }

  void VolumeHandler::setVolumeSinkMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    m_audioHandler->setSinkMute(device, mute);
    m_volumeSinkMute = mute;
    emit volumeSinkMuteChanged();
  }

  void VolumeHandler::setVolumeSourceMute(const bool mute) {
    QString device = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    m_audioHandler->setSourceMute(device, mute);
    m_volumeSourceMute = mute;
    emit volumeSourceMuteChanged();
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
    qInfo(lcVolume) << "Volume sink: " << m_volumeSink;
    return m_volumeSink;
  }

  void VolumeHandler::saveSettings() const {
    configuration_->save();
  }
}
