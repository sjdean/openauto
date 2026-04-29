#include "f1x/openauto/autoapp/UI/Monitor/PulseAudioHandler.hpp"
#ifdef Q_OS_LINUX
#include <qloggingcategory.h>
#include <pulse/introspect.h>
#include <pulse/operation.h>
#include <pulse/thread-mainloop.h>

#include "f1x/openauto/autoapp/UI/Monitor/IAudioHandler.h"

Q_LOGGING_CATEGORY(lcAudioPulse, "journeyos.audio.pulse")

namespace f1x::openauto::autoapp::UI::Monitor {

    PulseAudioHandler::PulseAudioHandler() {
        // Use PA_THREADED_MAINLOOP. This creates a background thread for audio processing.
        // This is crucial for Qt apps to avoid freezing the GUI.
        m_mainloop = pa_threaded_mainloop_new();
        if (!m_mainloop) {
            qCritical(lcAudioPulse) << "mainloop create failed";
            return;
        }

        m_api = pa_threaded_mainloop_get_api(m_mainloop);
        m_context = pa_context_new(m_api, "JourneyOS");

        pa_context_set_state_callback(m_context, context_state_callback, this);

        if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
            qCritical(lcAudioPulse) << "context connect failed";
            return;
        }

        // Start the background thread
        if (pa_threaded_mainloop_start(m_mainloop) < 0) {
            qCritical(lcAudioPulse) << "mainloop start failed";
            return;
        }
        
        // Wait for context to reach READY state
        while (true) {
            pa_threaded_mainloop_lock(m_mainloop);
            pa_context_state_t state = pa_context_get_state(m_context);
            pa_threaded_mainloop_unlock(m_mainloop);

            if (state == PA_CONTEXT_READY) break;
            if (!PA_CONTEXT_IS_GOOD(state)) {
                qCritical(lcAudioPulse) << "connection failed during init";
                return;
            }
            QThread::msleep(10);
        }

        // Subscribe to sink and source change events so AudioDeviceModel can
        // refresh its list automatically when hardware appears or disappears.
        pa_threaded_mainloop_lock(m_mainloop);
        pa_context_set_subscribe_callback(m_context, subscribe_callback, this);
        pa_operation *op = pa_context_subscribe(
            m_context,
            static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE),
            nullptr, nullptr);
        if (op) pa_operation_unref(op);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    PulseAudioHandler::~PulseAudioHandler() {
        if (m_mainloop) {
            pa_threaded_mainloop_stop(m_mainloop);
            if (m_context) {
                pa_context_disconnect(m_context);
                pa_context_unref(m_context);
            }
            pa_threaded_mainloop_free(m_mainloop);
        }
    }

    // --- HELPER CALLBACKS ---
    
    void PulseAudioHandler::context_state_callback(pa_context *c, void * /*userdata*/) {
        // Just logging state changes
        switch (pa_context_get_state(c)) {
            case PA_CONTEXT_READY: qInfo(lcAudioPulse) << "ready"; break;
            case PA_CONTEXT_FAILED: qWarning(lcAudioPulse) << "failed"; break;
            default: break;
        }
    }

    // --- VOLUME CONTROL ---

    void PulseAudioHandler::setSinkVolume(const QString& deviceName, int volume) {
        if (!m_mainloop || !m_context) return;
        int safeVolume = std::clamp(volume, 0, 255);
        pa_volume_t paVol = (pa_volume_t)((safeVolume * PA_VOLUME_NORM) / 255.0);

        pa_cvolume cv;
        cv.channels = 2; // Default to stereo
        for(int i=0; i<cv.channels; i++) cv.values[i] = paVol;

        const QString resolvedName = deviceName.isEmpty() ? getDefaultSink() : deviceName;
        if (resolvedName.isEmpty()) {
            qWarning(lcAudioPulse) << "setSinkVolume: no sink name available, skipping";
            return;
        }
        const QByteArray nameBytes = resolvedName.toUtf8();

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* o = pa_context_set_sink_volume_by_name(m_context, nameBytes.constData(), &cv, nullptr, nullptr);
        if (o) pa_operation_unref(o);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSourceVolume(const QString& deviceName, int volume) {
        if (!m_mainloop || !m_context) return;
        int safeVolume = std::clamp(volume, 0, 255);
        pa_volume_t paVol = (pa_volume_t)((safeVolume * PA_VOLUME_NORM) / 255.0);

        pa_cvolume cv;
        cv.channels = 1; // Mics usually mono
        for(int i=0; i<cv.channels; i++) cv.values[i] = paVol;

        const QString resolvedName = deviceName.isEmpty() ? getDefaultSource() : deviceName;
        if (resolvedName.isEmpty()) {
            qWarning(lcAudioPulse) << "setSourceVolume: no source name available, skipping";
            return;
        }
        const QByteArray nameBytes = resolvedName.toUtf8();

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* o = pa_context_set_source_volume_by_name(m_context, nameBytes.constData(), &cv, nullptr, nullptr);
        if (o) pa_operation_unref(o);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSinkMute(const QString& deviceName, bool mute) {
        if (!m_mainloop || !m_context) return;

        const QString resolvedName = deviceName.isEmpty() ? getDefaultSink() : deviceName;
        if (resolvedName.isEmpty()) {
            qWarning(lcAudioPulse) << "setSinkMute: no sink name available, skipping";
            return;
        }
        const QByteArray nameBytes = resolvedName.toUtf8();

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* o = pa_context_set_sink_mute_by_name(m_context, nameBytes.constData(), mute, nullptr, nullptr);
        if (o) pa_operation_unref(o);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSourceMute(const QString& deviceName, bool mute) {
        if (!m_mainloop || !m_context) return;

        const QString resolvedName = deviceName.isEmpty() ? getDefaultSource() : deviceName;
        if (resolvedName.isEmpty()) {
            qWarning(lcAudioPulse) << "setSourceMute: no source name available, skipping";
            return;
        }
        const QByteArray nameBytes = resolvedName.toUtf8();

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* o = pa_context_set_source_mute_by_name(m_context, nameBytes.constData(), mute, nullptr, nullptr);
        if (o) pa_operation_unref(o);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    // --- DEVICE INFORMATION (Synchronous Implementation) ---

    // Generic callback to populate our struct
    void PulseAudioHandler::GetSinkInfoCallback(pa_context *, const pa_sink_info *i, int eol, void *userdata) {
        ListDevicesState *state = static_cast<ListDevicesState*>(userdata);
        if (eol > 0) {
            state->finished = true;
            pa_threaded_mainloop_signal(state->loop, 0); // Wake up waiting thread
            return;
        }
        if (i) {
            EngineDevice device;
            device.description = QString::fromUtf8(i->description);
            device.value = QString::fromUtf8(i->name);
            device.iconname = device.GuessIconName();
            state->devices.append(device);
        }
    }

    EngineDeviceList PulseAudioHandler::getSinks() {
        if (!m_mainloop) return {};
        
        ListDevicesState state;
        state.loop = m_mainloop;

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation *op = pa_context_get_sink_info_list(m_context, &PulseAudioHandler::GetSinkInfoCallback, &state);
        
        if (op) {
            while (!state.finished) {
                pa_threaded_mainloop_wait(m_mainloop); // Wait for signal from callback
            }
            pa_operation_unref(op);
        }
        pa_threaded_mainloop_unlock(m_mainloop);

        return state.devices;
    }

    // Similar implementation for Sources
    void PulseAudioHandler::GetSourceInfoCallback(pa_context *, const pa_source_info *i, int eol, void *userdata) {
        ListDevicesState *state = static_cast<ListDevicesState*>(userdata);
        if (eol > 0) {
            state->finished = true;
            pa_threaded_mainloop_signal(state->loop, 0);
            return;
        }
        if (i) {
            EngineDevice device;
            device.description = QString::fromUtf8(i->description);
            device.value = QString::fromUtf8(i->name);
            state->devices.append(device);
        }
    }

    EngineDeviceList PulseAudioHandler::getSources() {
        if (!m_mainloop) return {};
        ListDevicesState state;
        state.loop = m_mainloop;

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation *op = pa_context_get_source_info_list(m_context, &PulseAudioHandler::GetSourceInfoCallback, &state);
        if (op) {
            while (!state.finished) pa_threaded_mainloop_wait(m_mainloop);
            pa_operation_unref(op);
        }
        pa_threaded_mainloop_unlock(m_mainloop);
        return state.devices;
    }
    
    // --- SERVER INFO (Default Sink Name) ---
    
    QString PulseAudioHandler::getDefaultSink() {
        if (!m_mainloop) return {};
        QString name;
        struct InfoState { pa_threaded_mainloop* loop; QString* name; bool finished = false; };
        InfoState state { m_mainloop, &name };

        auto cb = [](pa_context *, const pa_server_info *i, void *userdata) {
            InfoState *s = static_cast<InfoState*>(userdata);
            if (i) *s->name = QString::fromUtf8(i->default_sink_name);
            s->finished = true;
            pa_threaded_mainloop_signal(s->loop, 0);
        };

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* op = pa_context_get_server_info(m_context, cb, &state);
        if (op) {
             while (!state.finished) pa_threaded_mainloop_wait(m_mainloop);
             pa_operation_unref(op);
        }
        pa_threaded_mainloop_unlock(m_mainloop);
        return name;
    }

    QString PulseAudioHandler::getDefaultSource() {
        if (!m_mainloop) return {};
        QString name;
        struct InfoState { pa_threaded_mainloop* loop; QString* name; bool finished = false; };
        InfoState state { m_mainloop, &name };

        auto cb = [](pa_context *, const pa_server_info *i, void *userdata) {
            InfoState *s = static_cast<InfoState*>(userdata);
            if (i) *s->name = QString::fromUtf8(i->default_source_name);
            s->finished = true;
            pa_threaded_mainloop_signal(s->loop, 0);
        };

        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* op = pa_context_get_server_info(m_context, cb, &state);
        if (op) {
             while (!state.finished) pa_threaded_mainloop_wait(m_mainloop);
             pa_operation_unref(op);
        }
        pa_threaded_mainloop_unlock(m_mainloop);
        return name;
    }

    void PulseAudioHandler::addSinksChangedCallback(std::function<void()> cb) {
        m_sinksChangedCallbacks.push_back(std::move(cb));
    }

    void PulseAudioHandler::addSourcesChangedCallback(std::function<void()> cb) {
        m_sourcesChangedCallbacks.push_back(std::move(cb));
    }

    // Called from the PA mainloop thread (lock is held). Callbacks must not
    // call back into PA — they should only post to the Qt event loop.
    void PulseAudioHandler::subscribe_callback(pa_context*, pa_subscription_event_type_t t,
                                               uint32_t /*idx*/, void* userdata) {
        auto* self = static_cast<PulseAudioHandler*>(userdata);
        const auto facility = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
        const auto type     = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

        if (type != PA_SUBSCRIPTION_EVENT_NEW && type != PA_SUBSCRIPTION_EVENT_REMOVE)
            return;

        if (facility == PA_SUBSCRIPTION_EVENT_SINK) {
            for (auto& cb : self->m_sinksChangedCallbacks) cb();
        } else if (facility == PA_SUBSCRIPTION_EVENT_SOURCE) {
            for (auto& cb : self->m_sourcesChangedCallbacks) cb();
        }
    }

    std::vector<std::pair<std::string, std::string>> PulseAudioHandler::getDeviceList() {
        // Compatibility wrapper for your interface if needed
        std::vector<std::pair<std::string, std::string>> result;
        EngineDeviceList sinks = getSinks();
        for(const auto& dev : sinks) {
            result.push_back({dev.value.toStdString(), dev.description.toStdString()});
        }
        return result;
    }
}
#endif