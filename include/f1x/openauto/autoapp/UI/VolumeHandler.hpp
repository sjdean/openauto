#ifndef OPENAUTO_VOLUMEHANDLER_HPP
#define OPENAUTO_VOLUMEHANDLER_HPP

#include <QtCore/QObject>
#include <pulse/pulseaudio.h>
#include <cstdio>
#include <string>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/UI/PulseAudioHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  class VolumeHandler : public QObject {
    Q_OBJECT

    Q_PROPERTY(int volumeSink READ getVolumeSink WRITE setVolumeSink NOTIFY volumeSinkChanged)
    Q_PROPERTY(int volumeSource READ getVolumeSource WRITE setVolumeSource NOTIFY volumeSourceChanged)
    Q_PROPERTY(bool volumeSinkMute READ getVolumeSinkMute WRITE setVolumeSinkMute NOTIFY volumeSinkMuteChanged)
    Q_PROPERTY(bool volumeSourceMute READ getVolumeSourceMute WRITE setVolumeSourceMute NOTIFY volumeSourceMuteChanged)
  public:
    VolumeHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, PulseAudioHandler pulseAudioHandler);

    void setVolumeSinkMute(bool mute);
    void setVolumeSink(int volume);
    void setVolumeSourceMute(bool mute);
    void setVolumeSource(int volume);

    int getVolumeSink();
    int getVolumeSource();
    bool getVolumeSinkMute();
    bool getVolumeSourceMute();

    signals:
      void volumeSinkChanged();
      void volumeSourceChanged();
      void volumeSinkMuteChanged();
      void volumeSourceMuteChanged();

  private:
    f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;

    int m_volumeSink{};
    int m_volumeSource{};
    bool m_volumeSinkMute{};
    bool m_volumeSourceMute{};
    int calculateVolume(int min, int max, int target);

    PulseAudioHandler m_pulseAudioHandler;

  };

}

#endif//OPENAUTO_VOLUMEHANDLER_HPP