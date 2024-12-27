#include <f1x/openauto/autoapp/UI/VolumeHandler.hpp>

VolumeHandler::VolumeHandler() {

}

void VolumeHandler::onMute() {
  setMute(context, true);
}

void VolumeHandler::onUnMute() {
  setMute(context, false);
}

void VolumeHandler::onVolumeChange() {
  setVolume(context, volume);
}

std::string VolumeHandler::getDefaultSink(pa_context* context) {
  pa_operation* op;
  std::string defaultSinkName;
  pa_threaded_mainloop* loop = pa_threaded_mainloop_new();

  op = pa_context_get_server_info(context,
                                  [](pa_context*, const pa_server_info* i, void* data) {
                                    std::string* name = static_cast<std::string*>(data);
                                    *name = i->default_sink_name;
                                    pa_threaded_mainloop_signal((pa_threaded_mainloop*)pa_context_get_mainloop_api(pa_context_ref(pa_context_get_context(data)))->userdata, 0);
                                  }, &defaultSinkName);

  if (op) {
    pa_operation_unref(op);
    pa_threaded_mainloop_wait(loop);
  }
  pa_threaded_mainloop_free(loop);
  return defaultSinkName;
}

void VolumeHandler::setVolume(pa_context* context, int volume) {
  std::string sinkName = getDefaultSink(context);
  pa_cvolume cvolume;
  pa_cvolume_init(&cvolume);

  pa_channel_map map;
  pa_channel_map_init_auto(&map, 2, PA_CHANNEL_MAP_DEFAULT); // Assume stereo
  pa_cvolume_set(&cvolume, map.channels, PA_VOLUME_NORM * volume / 100);

  pa_operation* op = pa_context_set_sink_volume_by_name(context, sinkName.c_str(), &cvolume,
                                                        [](pa_context*, int success, void*) {
                                                          if (!success) {
                                                            fprintf(stderr, "Failed to set volume\n");
                                                          }
                                                        }, nullptr);

  if (op) {
    pa_operation_unref(op);
  }
}

void VolumeHandler::setMute(pa_context* context, bool mute) {
  std::string sinkName = getDefaultSink(context);
  pa_operation* op = pa_context_set_sink_mute_by_name(context, sinkName.c_str(), mute,
                                                      [](pa_context*, int success, void*) {
                                                        if (!success) {
                                                          fprintf(stderr, "Failed to set mute state\n");
                                                        }
                                                      }, nullptr);
  if (op) {
    pa_operation_unref(op);
  }
}