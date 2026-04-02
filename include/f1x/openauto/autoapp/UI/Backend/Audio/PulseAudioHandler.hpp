#pragma once

#include <QtCore/qglobal.h>   // defines Q_OS_LINUX before the guard below
#ifdef Q_OS_LINUX
#include "IAudioHandler.h"
#include <pulse/pulseaudio.h>
#include <QString>
#include <QVariant>
#include <QThread>

namespace f1x::openauto::autoapp::UI::Backend::Audio {

  class PulseAudioHandler : public IAudioHandler {
public:
    explicit PulseAudioHandler();
    ~PulseAudioHandler() override;

    QString getDefaultSink() override;
    QString getDefaultSource() override;

    void setSinkMute(const QString& deviceName, bool mute) override;
    void setSinkVolume(const QString& deviceName, int volume) override;
    void setSourceMute(const QString& deviceName, bool mute) override;
    void setSourceVolume(const QString& deviceName, int volume) override;

    EngineDeviceList getSinks() override;
    EngineDeviceList getSources() override;
    std::vector<std::pair<std::string, std::string>> getDeviceList() override;

  private:
    // Helper struct for synchronous callbacks
    struct ListDevicesState {
      bool finished = false;
      pa_threaded_mainloop* loop = nullptr;
      EngineDeviceList devices;
    };

    static void context_state_callback(pa_context *c, void *userdata);
    static void GetSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void GetSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata);

    pa_threaded_mainloop *m_mainloop = nullptr;
    pa_mainloop_api *m_api = nullptr;
    pa_context *m_context = nullptr;
  };
}
#endif