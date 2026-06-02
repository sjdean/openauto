#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include <algorithm>
#include <utility>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVolume, "journeyos.volume")

// TODO: Do we still need this? It was removed in v4.2

namespace f1x::openauto::autoapp::UI::Monitor  {

  VolumeHandler::VolumeHandler(configuration::IConfiguration::Pointer configuration,
                               std::shared_ptr<IAudioHandler> audioHandler) :
    configuration_(std::move(configuration)),
    m_audioHandler(std::move(audioHandler))
  {
    m_sinkMin   = configuration_->getSettingByName<int>("Audio", "PlaybackMin");
    m_sinkMax   = configuration_->getSettingByName<int>("Audio", "PlaybackMax");
    m_sourceMin = configuration_->getSettingByName<int>("Audio", "CaptureMin");
    m_sourceMax = configuration_->getSettingByName<int>("Audio", "CaptureMax");

    bool settingsChanged = false;

    // Resolve playback device — validates against live audio system, falls back to
    // PA default if the stored name is missing or stale.
    // NOTE: only persist back to config when the stored value was empty (no user
    // preference).  If the user has chosen a specific sink (e.g. IQaudIODAC) and
    // that sink isn't available yet at startup (PA timing race), we must NOT
    // overwrite their preference with the fallback default.
    const QString storedSink = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    m_resolvedSinkName = m_audioHandler->resolveSinkName(storedSink);
    if (storedSink.isEmpty() && !m_resolvedSinkName.isEmpty()) {
        configuration_->updateSettingByName<QString>("Audio", "PlaybackDevice", m_resolvedSinkName);
        settingsChanged = true;
    }
    qInfo(lcVolume) << "PlaybackDevice: stored=" << storedSink << "resolved=" << m_resolvedSinkName;
    // Make the resolved sink the PA default so QMediaDevices::defaultAudioOutput()
    // returns it — QtAudioOutput falls back to the PA default when no id match found.
    m_audioHandler->setDefaultSink(m_resolvedSinkName);

    const int storedSlider = std::clamp(
        configuration_->getSettingByName<int>("Audio", "PlaybackVolume"), 0, 255);
    const int initPaValue = std::clamp(m_sinkMin + (m_sinkMax - m_sinkMin) * storedSlider / 255, 0, 255);
    m_audioHandler->setSinkVolume(m_resolvedSinkName, initPaValue);
    m_volumeSink = storedSlider;

    // Resolve capture device — same policy: only persist if no preference was stored.
    const QString storedSource = configuration_->getSettingByName<QString>("Audio", "CaptureDevice");
    m_resolvedSourceName = m_audioHandler->resolveSourceName(storedSource);
    if (storedSource.isEmpty() && !m_resolvedSourceName.isEmpty()) {
        configuration_->updateSettingByName<QString>("Audio", "CaptureDevice", m_resolvedSourceName);
        settingsChanged = true;
    }
    qInfo(lcVolume) << "CaptureDevice: stored=" << storedSource << "resolved=" << m_resolvedSourceName;

    const int storedCaptureSlider = std::clamp(
        configuration_->getSettingByName<int>("Audio", "CaptureVolume"), 0, 255);
    const int initCaptureValue = std::clamp(m_sourceMin + (m_sourceMax - m_sourceMin) * storedCaptureSlider / 255, 0, 255);
    m_audioHandler->setSourceVolume(m_resolvedSourceName, initCaptureValue);
    m_volumeSource = storedCaptureSlider;

    if (settingsChanged)
        configuration_->save();
  }

  // --- Device resolution (called when user selects a new device in Settings) ---

  void VolumeHandler::updatePlaybackDevice(const QString& name) {
    m_resolvedSinkName = m_audioHandler->resolveSinkName(name);
    qInfo(lcVolume) << "updatePlaybackDevice: requested=" << name
                    << "resolved=" << m_resolvedSinkName;
    m_audioHandler->setDefaultSink(m_resolvedSinkName);
  }

  void VolumeHandler::updateCaptureDevice(const QString& name) {
    m_resolvedSourceName = m_audioHandler->resolveSourceName(name);
    qInfo(lcVolume) << "updateCaptureDevice: requested=" << name
                    << "resolved=" << m_resolvedSourceName;
  }

  // --- Bounds (updated from Settings) ---

  void VolumeHandler::updatePlaybackBounds(int min, int max) {
    m_sinkMin = min;
    m_sinkMax = max;
    qInfo(lcVolume) << "playback bounds updated min=" << min << "max=" << max;
  }

  void VolumeHandler::updateCaptureBounds(int min, int max) {
    m_sourceMin = min;
    m_sourceMax = max;
    qInfo(lcVolume) << "capture bounds updated min=" << min << "max=" << max;
  }

  // --- Volume / mute setters (use cached resolved names — no runtime lookup) ---

  void VolumeHandler::setVolumeSink(const int sliderValue) {
    if (m_resolvedSinkName.isEmpty()) {
        qWarning(lcVolume) << "setVolumeSink: no resolved sink, skipping";
        return;
    }
    const int slider  = std::clamp(sliderValue, 0, 255);
    const int paValue = std::clamp(m_sinkMin + (m_sinkMax - m_sinkMin) * slider / 255, 0, 255);
    qInfo(lcVolume) << "setVolumeSink slider=" << slider
                    << "range=[" << m_sinkMin << "," << m_sinkMax << "] pa=" << paValue
                    << "sink=" << m_resolvedSinkName;
    m_audioHandler->setSinkVolume(m_resolvedSinkName, paValue);
    configuration_->updateSettingByName("Audio", "PlaybackVolume", slider);
    m_volumeSink = slider;
    emit volumeSinkChanged();
  }

  void VolumeHandler::setVolumeSource(const int sliderValue) {
    if (m_resolvedSourceName.isEmpty()) return;
    const int slider  = std::clamp(sliderValue, 0, 255);
    const int paValue = std::clamp(m_sourceMin + (m_sourceMax - m_sourceMin) * slider / 255, 0, 255);
    m_audioHandler->setSourceVolume(m_resolvedSourceName, paValue);
    configuration_->updateSettingByName("Audio", "CaptureVolume", slider);
    m_volumeSource = slider;
    emit volumeSourceChanged();
  }

  void VolumeHandler::setVolumeSinkMute(const bool mute) {
    if (m_resolvedSinkName.isEmpty()) return;
    m_audioHandler->setSinkMute(m_resolvedSinkName, mute);
    m_volumeSinkMute = mute;
    emit volumeSinkMuteChanged();
  }

  void VolumeHandler::setVolumeSourceMute(const bool mute) {
    if (m_resolvedSourceName.isEmpty()) return;
    m_audioHandler->setSourceMute(m_resolvedSourceName, mute);
    m_volumeSourceMute = mute;
    emit volumeSourceMuteChanged();
  }

  void VolumeHandler::toggleSinkMute() {
    setVolumeSinkMute(!m_volumeSinkMute);
  }

  void VolumeHandler::reapplyVolume() {
    setVolumeSink(m_volumeSink);
  }

  // --- Getters ---

  int  VolumeHandler::getVolumeSink()       const { return m_volumeSink; }
  int  VolumeHandler::getVolumeSource()     const { return m_volumeSource; }
  bool VolumeHandler::getVolumeSinkMute()   const { return m_volumeSinkMute; }
  bool VolumeHandler::getVolumeSourceMute() const { return m_volumeSourceMute; }
  int  VolumeHandler::getVolumeSinkMin()    const { return m_sinkMin; }
  int  VolumeHandler::getVolumeSinkMax()    const { return m_sinkMax; }
  int  VolumeHandler::getVolumeSourceMin()  const { return m_sourceMin; }
  int  VolumeHandler::getVolumeSourceMax()  const { return m_sourceMax; }

  void VolumeHandler::saveSettings() const { configuration_->save(); }

} // namespace