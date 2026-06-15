#include "f1x/openauto/autoapp/UI/ViewModel/VolumeViewModel.hpp"
#include <algorithm>
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

namespace f1x::openauto::autoapp::UI::ViewModel {
using configuration::ConfigGroup;
using configuration::ConfigKey;

  VolumeViewModel::VolumeViewModel(configuration::IConfiguration::Pointer configuration,
                                   std::shared_ptr<Backend::Audio::IAudioHandler> audioHandler) :
    configuration_(std::move(configuration)),
    m_audioHandler(std::move(audioHandler))
  {
    m_sinkMin   = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMin);
    m_sinkMax   = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackMax);
    m_sourceMin = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMin);
    m_sourceMax = configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureMax);

    bool settingsChanged = false;

    // Resolve playback device — validates against live audio system, falls back to
    // PA default if the stored name is missing or stale.
    // NOTE: only persist back to config when the stored value was empty (no user
    // preference). If the user has chosen a specific sink (e.g. IQaudIODAC) and
    // that sink isn't available yet at startup (PA timing race), we must NOT
    // overwrite their preference with the fallback default.
    const QString storedSink = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice);
    m_resolvedSinkName = m_audioHandler->resolveSinkName(storedSink);
    if (storedSink.isEmpty() && !m_resolvedSinkName.isEmpty()) {
        configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioPlaybackDevice, m_resolvedSinkName);
        settingsChanged = true;
    }
    qInfo(lcVolume) << "PlaybackDevice: stored=" << storedSink << "resolved=" << m_resolvedSinkName;
    m_audioHandler->setDefaultSink(m_resolvedSinkName);

    const int storedVolume = std::clamp(
        configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume), m_sinkMin, m_sinkMax);
    m_audioHandler->setSinkVolume(m_resolvedSinkName, storedVolume);
    m_volumeSink = storedVolume;

    // Resolve capture device — same policy: only persist if no preference was stored.
    const QString storedSource = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice);
    m_resolvedSourceName = m_audioHandler->resolveSourceName(storedSource);
    if (storedSource.isEmpty() && !m_resolvedSourceName.isEmpty()) {
        configuration_->updateSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice, m_resolvedSourceName);
        settingsChanged = true;
    }
    qInfo(lcVolume) << "CaptureDevice: stored=" << storedSource << "resolved=" << m_resolvedSourceName;

    const int storedCaptureVolume = std::clamp(
        configuration_->getSettingByName<int>(ConfigGroup::Audio, ConfigKey::AudioCaptureVolume), m_sourceMin, m_sourceMax);
    m_audioHandler->setSourceVolume(m_resolvedSourceName, storedCaptureVolume);
    m_volumeSource = storedCaptureVolume;

    if (settingsChanged)
        configuration_->save();
  }

  // --- Device resolution (called when user selects a new device in Settings) ---

  void VolumeViewModel::updatePlaybackDevice(const QString& name) {
    m_resolvedSinkName = m_audioHandler->resolveSinkName(name);
    qInfo(lcVolume) << "updatePlaybackDevice: requested=" << name << "resolved=" << m_resolvedSinkName;
    m_audioHandler->setDefaultSink(m_resolvedSinkName);
  }

  void VolumeViewModel::updateCaptureDevice(const QString& name) {
    m_resolvedSourceName = m_audioHandler->resolveSourceName(name);
    qInfo(lcVolume) << "updateCaptureDevice: requested=" << name << "resolved=" << m_resolvedSourceName;
  }

  // --- Bounds (updated from Settings) ---

  void VolumeViewModel::updatePlaybackBounds(int min, int max) {
    m_sinkMin = min;
    m_sinkMax = max;
    qInfo(lcVolume) << "playback bounds updated min=" << min << "max=" << max;
  }

  void VolumeViewModel::updateCaptureBounds(int min, int max) {
    m_sourceMin = min;
    m_sourceMax = max;
    qInfo(lcVolume) << "capture bounds updated min=" << min << "max=" << max;
  }

  // --- Volume / mute setters ---

  void VolumeViewModel::setVolumeSink(const int volume) {
    if (m_resolvedSinkName.isEmpty()) {
        qWarning(lcVolume) << "setVolumeSink: no resolved sink, skipping";
        return;
    }
    const int clamped = std::clamp(volume, m_sinkMin, m_sinkMax);
    m_audioHandler->setSinkVolume(m_resolvedSinkName, clamped);
    configuration_->updateSettingByName(ConfigGroup::Audio, ConfigKey::AudioPlaybackVolume, clamped);
    m_volumeSink = clamped;
    emit volumeSinkChanged();
  }

  void VolumeViewModel::setVolumeSource(const int volume) {
    if (m_resolvedSourceName.isEmpty()) return;
    const int clamped = std::clamp(volume, m_sourceMin, m_sourceMax);
    m_audioHandler->setSourceVolume(m_resolvedSourceName, clamped);
    configuration_->updateSettingByName(ConfigGroup::Audio, ConfigKey::AudioCaptureVolume, clamped);
    m_volumeSource = clamped;
    emit volumeSourceChanged();
  }

  void VolumeViewModel::setVolumeSinkMute(const bool mute) {
    if (m_resolvedSinkName.isEmpty()) return;
    m_audioHandler->setSinkMute(m_resolvedSinkName, mute);
    m_volumeSinkMute = mute;
    emit volumeSinkMuteChanged();
  }

  void VolumeViewModel::setVolumeSourceMute(const bool mute) {
    if (m_resolvedSourceName.isEmpty()) return;
    m_audioHandler->setSourceMute(m_resolvedSourceName, mute);
    m_volumeSourceMute = mute;
    emit volumeSourceMuteChanged();
  }

  void VolumeViewModel::toggleSinkMute() {
    setVolumeSinkMute(!m_volumeSinkMute);
  }

  void VolumeViewModel::reapplyVolume() {
    setVolumeSink(m_volumeSink);
  }

  // --- Getters ---

  int  VolumeViewModel::getVolumeSink()       const { return m_volumeSink; }
  int  VolumeViewModel::getVolumeSource()     const { return m_volumeSource; }
  bool VolumeViewModel::getVolumeSinkMute()   const { return m_volumeSinkMute; }
  bool VolumeViewModel::getVolumeSourceMute() const { return m_volumeSourceMute; }
  int  VolumeViewModel::getVolumeSinkMin()    const { return m_sinkMin; }
  int  VolumeViewModel::getVolumeSinkMax()    const { return m_sinkMax; }
  int  VolumeViewModel::getVolumeSourceMin()  const { return m_sourceMin; }
  int  VolumeViewModel::getVolumeSourceMax()  const { return m_sourceMax; }

  void VolumeViewModel::saveSettings() const { configuration_->save(); }

} // namespace