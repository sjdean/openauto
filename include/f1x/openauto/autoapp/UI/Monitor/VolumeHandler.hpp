#pragma once

#include <QObject>
#include "IAudioHandler.h"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {
  class VolumeHandler : public QObject {
    Q_OBJECT

    Q_PROPERTY(int volumeSink READ getVolumeSink WRITE setVolumeSink NOTIFY volumeSinkChanged)
    Q_PROPERTY(int volumeSource READ getVolumeSource WRITE setVolumeSource NOTIFY volumeSourceChanged)
    Q_PROPERTY(bool volumeSinkMute READ getVolumeSinkMute WRITE setVolumeSinkMute NOTIFY volumeSinkMuteChanged)
    Q_PROPERTY(bool volumeSourceMute READ getVolumeSourceMute WRITE setVolumeSourceMute NOTIFY volumeSourceMuteChanged)
  public:
    VolumeHandler(configuration::IConfiguration::Pointer configuration, std::shared_ptr<IAudioHandler> audioHandler);

    Q_INVOKABLE void saveSettings() const;

    void setVolumeSinkMute(bool mute);
    void setVolumeSink(int volume);
    void setVolumeSourceMute(bool mute);
    void setVolumeSource(int volume);

    int getVolumeSink() const;
    int getVolumeSource() const;
    bool getVolumeSinkMute() const;
    bool getVolumeSourceMute() const;

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

    static int calculateVolume(int min, int max, int target);

    std::shared_ptr<IAudioHandler> m_audioHandler;

  };

}

