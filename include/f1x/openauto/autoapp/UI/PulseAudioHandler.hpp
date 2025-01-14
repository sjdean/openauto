#ifndef OPENAUTO_PULSEAUDIOHANDLER_HPP
#define OPENAUTO_PULSEAUDIOHANDLER_HPP

#include <pulse/pulseaudio.h>
#include <QString>

namespace f1x::openauto::autoapp::UI {
  class PulseAudioHandler {

  public:
    explicit PulseAudioHandler();
    QString getDefaultSink();
    QString getDefaultSource();

    void setSinkMute(bool mute);
    void setSinkVolume(int volume);
    void setSourceMute(bool mute);
    void setSourceVolume(int volume);

    std::vector<std::pair<std::string, std::string>> getDeviceList();

  private:
    pa_mainloop *m_mainloop;
    pa_mainloop_api *m_mainloop_api;
    pa_context *m_context;
  };
}
#endif//OPENAUTO_PULSEAUDIOHANDLER_HPP