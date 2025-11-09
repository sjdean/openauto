#include <f1x/openauto/autoapp/UI/Monitor/PulseAudioHandler.hpp>
#include <string>

namespace f1x::openauto::autoapp::UI::Monitor  {
  PulseAudioHandler::PulseAudioHandler() {
    m_mainloop = pa_mainloop_new();
    m_mainloop_api = pa_mainloop_get_api(m_mainloop);

    m_context = pa_context_new(m_mainloop_api, "JourneyOS");
    pa_context_set_state_callback(m_context,
                                  [](pa_context *c, void *userdata) {
                                    if (pa_context_get_state(c) == PA_CONTEXT_READY) {
                                      pa_threaded_mainloop_signal((pa_threaded_mainloop *) userdata, 0);
                                    }
                                  }, m_mainloop);

    if (pa_context_connect(m_context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
      // throw error
      return;
    }

    pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(loop);

    // Wait for the context to be ready
    if (pa_context_get_state(m_context) != PA_CONTEXT_READY) {
      pa_threaded_mainloop_wait(loop);
    }

    pa_threaded_mainloop_free(loop);
  }

  QString PulseAudioHandler::getDefaultSink() {
    pa_operation *op;
    QString defaultSinkName;

    op = pa_context_get_server_info(m_context,
        // Cast the lambda to pa_server_info_cb_t
                                    static_cast<pa_server_info_cb_t>([](pa_context *context, const pa_server_info *info, void *userdata) {
                                      std::string *name = static_cast<std::string *>(userdata);
                                      *name = info->default_sink_name;
                                      pa_threaded_mainloop_signal((pa_threaded_mainloop *) userdata, 0);
                                    }),
                                    &defaultSinkName);

    if (op) {
      pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
      pa_threaded_mainloop_start(loop);

      // Wait for the context to be ready
      if (pa_context_get_state(m_context) != PA_CONTEXT_READY) {
        pa_threaded_mainloop_wait(loop);
      }

      pa_threaded_mainloop_free(loop);
      pa_operation_unref(op);
    } else {
      // Handle the error, e.g., log it or throw an exception
      throw std::runtime_error("Failed to get server info for default sink");
    }

    return defaultSinkName;
  }

  void PulseAudioHandler::GetSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {

    Q_UNUSED(c);

    ListDevicesState *state = reinterpret_cast<ListDevicesState*>(userdata);
    if (!state) return;

    if (i) {
      EngineDevice device;
      device.description = QString::fromUtf8(i->description);
      device.value = QString::fromUtf8(i->name);
      device.iconname = device.GuessIconName();

      state->devices.append(device);
    }

    if (eol > 0) {
      state->finished = true;
    }
  }

  EngineDeviceList PulseAudioHandler::getSinks() {
    if (!m_context || pa_context_get_state(m_context) != PA_CONTEXT_READY) {
      // Log error or attempt reconnection
      return EngineDeviceList();
    }

    ListDevicesState state;
    pa_operation *op = pa_context_get_sink_info_list(m_context, &PulseAudioHandler::GetSinkInfoCallback, &state);

    if (op) {
      pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
      pa_threaded_mainloop_start(loop);

      // Wait for the callback to finish
      while (!state.finished) {
        pa_threaded_mainloop_wait(loop);
      }

      pa_threaded_mainloop_free(loop);
      pa_operation_unref(op);
    } else {
      // Handle the error, e.g., log it or throw an exception
      throw std::runtime_error("Failed to get sink list");
    }

    return state.devices;
  }

  void PulseAudioHandler::GetSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata) {

    Q_UNUSED(c);

    ListDevicesState *state = reinterpret_cast<ListDevicesState*>(userdata);
    if (!state) return;

    if (i) {
      EngineDevice device;
      device.description = QString::fromUtf8(i->description);
      device.value = QString::fromUtf8(i->name);
      device.iconname = device.GuessIconName();

      state->devices.append(device);
    }

    if (eol > 0) {
      state->finished = true;
    }
  }

  EngineDeviceList PulseAudioHandler::getSources() {
    if (!m_context || pa_context_get_state(m_context) != PA_CONTEXT_READY) {
      // Log error or attempt reconnection
      return EngineDeviceList();
    }

    ListDevicesState state;
    pa_operation *op = pa_context_get_source_info_list(m_context, &PulseAudioHandler::GetSourceInfoCallback, &state);

    if (op) {
      pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
      pa_threaded_mainloop_start(loop);

      // Wait for the callback to finish
      while (!state.finished) {
        pa_threaded_mainloop_wait(loop);
      }

      pa_threaded_mainloop_free(loop);
      pa_operation_unref(op);
    } else {
      // Handle the error, e.g., log it or throw an exception
      throw std::runtime_error("Failed to get sink list");
    }

    return state.devices;
  }

  QString PulseAudioHandler::getDefaultSource() {
    pa_operation *op;
    QString defaultSourceName;

    op = pa_context_get_server_info(m_context,
        // Cast the lambda to pa_server_info_cb_t
                                    static_cast<pa_server_info_cb_t>([](pa_context *context, const pa_server_info *info, void *userdata) {
                                      std::string *name = static_cast<std::string *>(userdata);
                                      *name = info->default_source_name;
                                      pa_threaded_mainloop_signal((pa_threaded_mainloop *) userdata, 0);
                                    }),
                                    &defaultSourceName);

    if (op) {
      pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
      pa_threaded_mainloop_start(loop);

      // Wait for the context to be ready
      if (pa_context_get_state(m_context) != PA_CONTEXT_READY) {
        pa_threaded_mainloop_wait(loop);
      }

      pa_threaded_mainloop_free(loop);
      pa_operation_unref(op);
    } else {
      // Handle the error, e.g., log it or throw an exception
      throw std::runtime_error("Failed to get server info for default source");
    }

    return defaultSourceName;
  }

  void PulseAudioHandler::setSinkVolume(int volume) {
    // TODO: Swap to Configured Sink
    QString sinkName = getDefaultSink();
    pa_cvolume cvolume;
    pa_cvolume_init(&cvolume);

    pa_channel_map map;
    pa_channel_map_init_auto(&map, 2, PA_CHANNEL_MAP_DEFAULT); // Assume stereo
    pa_cvolume_set(&cvolume, map.channels, PA_VOLUME_NORM * volume / 100);

    pa_operation *op = pa_context_set_sink_volume_by_name(m_context, sinkName.toStdString().c_str(), &cvolume,
                                                          [](pa_context *, int success, void *) {
                                                            if (!success) {
                                                              fprintf(stderr, "Failed to set volume\n");
                                                            }
                                                          }, nullptr);

    if (op) {
      pa_operation_unref(op);
    }
  }

  void PulseAudioHandler::setSourceVolume(int volume) {
    // TODO: Swap to Configured Source
    QString sinkName = getDefaultSource();
    pa_cvolume cvolume;
    pa_cvolume_init(&cvolume);

    pa_channel_map map;
    pa_channel_map_init_auto(&map, 2, PA_CHANNEL_MAP_DEFAULT); // Assume stereo
    pa_cvolume_set(&cvolume, map.channels, PA_VOLUME_NORM * volume / 100);

    pa_operation *op = pa_context_set_sink_volume_by_name(m_context, sinkName.toStdString().c_str(), &cvolume,
                                                          [](pa_context *, int success, void *) {
                                                            if (!success) {
                                                              fprintf(stderr, "Failed to set volume\n");
                                                            }
                                                          }, nullptr);

    if (op) {
      pa_operation_unref(op);
    }
  }

  void PulseAudioHandler::setSinkMute(bool mute) {
    // TODO: Swap to Configured Sink
    QString sinkName = getDefaultSink();
    pa_operation *op = pa_context_set_sink_mute_by_name(m_context, sinkName.toStdString().c_str(), mute,
                                                        [](pa_context *, int success, void *) {
                                                          if (!success) {
                                                            fprintf(stderr, "Failed to set mute state\n");
                                                          }
                                                        }, nullptr);
    if (op) {
      pa_operation_unref(op);
    }
  }

  void PulseAudioHandler::setSourceMute(bool mute) {
    // TODO: Swap to Configured Soure
    QString sinkName = getDefaultSink();
    pa_operation *op = pa_context_set_sink_mute_by_name(m_context, sinkName.toStdString().c_str(), mute,
                                                        [](pa_context *, int success, void *) {
                                                          if (!success) {
                                                            fprintf(stderr, "Failed to set mute state\n");
                                                          }
                                                        }, nullptr);
    if (op) {
      pa_operation_unref(op);
    }
  }

  std::vector<std::pair<std::string, std::string>> PulseAudioHandler::getDeviceList() {
    std::vector<std::pair<std::string, std::string>> devices;

    // Callback for collecting device info
    auto info_callback = [](pa_context *c, const pa_source_info *i, int eol, void *userdata) {
      auto *devices = static_cast<std::vector<std::pair<std::string, std::string>> *>(userdata);

      if (eol > 0) return; // End of list

      if (i) {
        std::string name = i->name ? i->name : "Unknown";
        std::string description = i->description ? i->description : "No description";
        devices->emplace_back(name, description);
      }
    };

    pa_operation *op = pa_context_get_source_info_list(m_context, info_callback, &devices);
    if (op) {
      while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_mainloop_iterate(m_mainloop, 1, NULL);
      }
      pa_operation_unref(op); // Don't forget to unref the operation after use
    } else {
      // Handle the error, maybe log it or inform the user
      fprintf(stderr, "Failed to get source info list\n");
    }

    return devices;
  }
}