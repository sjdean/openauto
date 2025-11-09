#ifndef OPENAUTO_PULSEAUDIOHANDLER_HPP
#define OPENAUTO_PULSEAUDIOHANDLER_HPP

#include <pulse/pulseaudio.h>
#include <QString>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Monitor {
  class EngineDevice {
  public:
    explicit EngineDevice();
    QString GuessIconName() const;
    QString description;
    QString value;
    QString iconname;
    int card;
    int device;
  };
  using EngineDeviceList = QList<EngineDevice>;

  class PulseAudioHandler {

  public:
    explicit PulseAudioHandler();
    QString getDefaultSink();
    QString getDefaultSource();

    void setSinkMute(bool mute);
    void setSinkVolume(int volume);
    void setSourceMute(bool mute);
    void setSourceVolume(int volume);

    EngineDeviceList getSinks();
    EngineDeviceList getSources();
    std::vector<std::pair<std::string, std::string>> getDeviceList();

  private:
    struct ListDevicesState {
      ListDevicesState() : finished(false) {}

      bool finished;
      EngineDeviceList devices;
    };

    static void GetSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void GetSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata);


    pa_mainloop *m_mainloop;
    pa_mainloop_api *m_mainloop_api;
    pa_context *m_context;
  };
}
#endif//OPENAUTO_PULSEAUDIOHANDLER_HPP