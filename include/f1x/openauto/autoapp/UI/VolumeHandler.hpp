#include <pulse/pulseaudio.h>
#include <cstdio>
#include <string>

class VolumeHandler {
public:
  VolumeHandler();

  std::string getDefaultSink(pa_context* context);
  void setMute(pa_context* context, bool mute);
  void setVolume(pa_context* context, int volume);

  void onMute();
  void onUnMute();
  void onVolumeChange();
};