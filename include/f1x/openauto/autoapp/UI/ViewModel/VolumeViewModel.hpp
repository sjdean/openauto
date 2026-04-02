#pragma once

#include <QObject>
#include "f1x/openauto/autoapp/UI/Backend/Audio/IAudioHandler.h"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
  class VolumeViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(int volumeSink READ getVolumeSink WRITE setVolumeSink NOTIFY volumeSinkChanged)
    Q_PROPERTY(int volumeSource READ getVolumeSource WRITE setVolumeSource NOTIFY volumeSourceChanged)
    Q_PROPERTY(bool volumeSinkMute READ getVolumeSinkMute WRITE setVolumeSinkMute NOTIFY volumeSinkMuteChanged)
    Q_PROPERTY(bool volumeSourceMute READ getVolumeSourceMute WRITE setVolumeSourceMute NOTIFY volumeSourceMuteChanged)
    Q_PROPERTY(int volumeSinkMin READ getVolumeSinkMin CONSTANT)
    Q_PROPERTY(int volumeSinkMax READ getVolumeSinkMax CONSTANT)
    Q_PROPERTY(int volumeSourceMin READ getVolumeSourceMin CONSTANT)
    Q_PROPERTY(int volumeSourceMax READ getVolumeSourceMax CONSTANT)
  public:
    VolumeViewModel(configuration::IConfiguration::Pointer configuration, std::shared_ptr<f1x::openauto::autoapp::UI::Backend::Audio::IAudioHandler> audioHandler);

    Q_INVOKABLE void saveSettings() const;

    void setVolumeSinkMute(bool mute);
    void setVolumeSink(int volume);
    void setVolumeSourceMute(bool mute);
    void setVolumeSource(int volume);

    int getVolumeSink() const;
    int getVolumeSource() const;
    bool getVolumeSinkMute() const;
    bool getVolumeSourceMute() const;
    int getVolumeSinkMin() const;
    int getVolumeSinkMax() const;
    int getVolumeSourceMin() const;
    int getVolumeSourceMax() const;

    signals:
      void volumeSinkChanged();
      void volumeSourceChanged();
      void volumeSinkMuteChanged();
      void volumeSourceMuteChanged();

  private:
    configuration::IConfiguration::Pointer configuration_;

    int m_volumeSink{};
    int m_volumeSource{};
    bool m_volumeSinkMute{};
    bool m_volumeSourceMute{};
    int m_sinkMin{0};
    int m_sinkMax{255};
    int m_sourceMin{0};
    int m_sourceMax{255};

    std::shared_ptr<f1x::openauto::autoapp::UI::Backend::Audio::IAudioHandler> m_audioHandler;

  };

}

