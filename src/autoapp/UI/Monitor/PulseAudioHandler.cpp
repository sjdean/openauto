#include "f1x/openauto/autoapp/UI/Monitor/PulseAudioHandler.hpp"
#ifdef Q_OS_LINUX
#include <cstdlib>
#include <qloggingcategory.h>
#include <QScopeGuard>
#include <pulse/introspect.h>
#include <pulse/operation.h>
#include <pulse/thread-mainloop.h>
#include <alsa/asoundlib.h>

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

    // --- NAME RESOLUTION ---

    // Returns the PA default sink name.  Used on first boot when no device is
    // stored in config.  PA sets the default via set-default-sink in
    // journeyos-iqaudio.pa, so this reliably returns the IQaudIO DAC sink.
    // Must be called WITHOUT holding the mainloop lock (getDefaultSink acquires it).
    QString PulseAudioHandler::bestAvailableSink() {
        const QString def = getDefaultSink();
        qInfo(lcAudioPulse) << "bestAvailableSink:" << def;
        return def;
    }

    void PulseAudioHandler::setDefaultSink(const QString& sinkName) {
        if (!m_mainloop || !m_context || sinkName.isEmpty()) return;
        const QByteArray nb = sinkName.toUtf8();

        // 1. Tell PA which sink is the default
        pa_threaded_mainloop_lock(m_mainloop);
        qInfo(lcAudioPulse) << "setDefaultSink" << sinkName;
        pa_operation* op = pa_context_set_default_sink(m_context, nb.constData(), nullptr, nullptr);
        if (op) pa_operation_unref(op);
        pa_threaded_mainloop_unlock(m_mainloop);

        // 2. Query sink info: grab ALSA device.string and channel count
        struct SinkInfo { pa_threaded_mainloop* loop; QString device; uint8_t ch{2}; bool done{false}; };
        SinkInfo si{m_mainloop};
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* qop = pa_context_get_sink_info_by_name(m_context, nb.constData(),
            [](pa_context*, const pa_sink_info* i, int eol, void* ud) {
                auto* s = static_cast<SinkInfo*>(ud);
                if (eol > 0) { s->done = true; pa_threaded_mainloop_signal(s->loop, 0); return; }
                if (!i) return;
                const char* dev = pa_proplist_gets(i->proplist, "device.string");
                if (dev && *dev) s->device = QString::fromUtf8(dev);
                s->ch = i->channel_map.channels;
            }, &si);
        if (qop) { while (!si.done) pa_threaded_mainloop_wait(m_mainloop); pa_operation_unref(qop); }
        pa_threaded_mainloop_unlock(m_mainloop);

        if (!si.device.isEmpty()) {
            m_alsaCardDevice = si.device;
            qInfo(lcAudioPulse) << "setDefaultSink: ALSA card device=" << m_alsaCardDevice;
        } else {
            qWarning(lcAudioPulse) << "setDefaultSink: no ALSA device.string for" << sinkName
                                   << "— volume will use PA soft-volume fallback";
        }

        // 3. Lock PA sink at 100% — volume is now driven by ALSA hardware mixer.
        //    Soft-volume multiply at anything < 100% silently discards low-order bits
        //    (reduces effective bit depth), so we keep the PA path bit-perfect.
        pa_threaded_mainloop_lock(m_mainloop);
        pa_cvolume cv;
        pa_cvolume_set(&cv, si.ch, PA_VOLUME_NORM);
        pa_operation* vop = pa_context_set_sink_volume_by_name(
            m_context, nb.constData(), &cv, nullptr, nullptr);
        if (vop) pa_operation_unref(vop);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    QString PulseAudioHandler::resolveSinkName(const QString& requested) {
        if (!m_mainloop || !m_context) return {};
        const QString candidate = requested.isEmpty() ? bestAvailableSink() : requested;
        if (candidate.isEmpty()) {
            qWarning(lcAudioPulse) << "resolveSinkName: no sinks available in PA";
            return {};
        }
        struct State { pa_threaded_mainloop* loop; bool found{false}; bool done{false}; };
        State s{m_mainloop};
        const QByteArray nb = candidate.toUtf8();
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* op = pa_context_get_sink_info_by_name(m_context, nb.constData(),
            [](pa_context*, const pa_sink_info* i, int eol, void* ud) {
                auto* s = static_cast<State*>(ud);
                if (eol > 0 || !i) { s->done = true; pa_threaded_mainloop_signal(s->loop, 0); return; }
                s->found = true;
            }, &s);
        if (op) { while (!s.done) pa_threaded_mainloop_wait(m_mainloop); pa_operation_unref(op); }
        pa_threaded_mainloop_unlock(m_mainloop);

        if (s.found) {
            qInfo(lcAudioPulse) << "resolveSinkName:" << candidate << "verified";
            return candidate;
        }
        // Requested sink not present — try the PA default.
        if (requested.isEmpty()) return {};  // already tried default above
        qWarning(lcAudioPulse) << "resolveSinkName: sink" << candidate
                               << "not found in PA, falling back to default";
        return getDefaultSink();
    }

    QString PulseAudioHandler::resolveSourceName(const QString& requested) {
        if (!m_mainloop || !m_context) return {};
        const QString candidate = requested.isEmpty() ? getDefaultSource() : requested;
        if (candidate.isEmpty()) {
            qWarning(lcAudioPulse) << "resolveSourceName: no sources available in PA";
            return {};
        }
        struct State { pa_threaded_mainloop* loop; bool found{false}; bool done{false}; };
        State s{m_mainloop};
        const QByteArray nb = candidate.toUtf8();
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* op = pa_context_get_source_info_by_name(m_context, nb.constData(),
            [](pa_context*, const pa_source_info* i, int eol, void* ud) {
                auto* s = static_cast<State*>(ud);
                if (eol > 0 || !i) { s->done = true; pa_threaded_mainloop_signal(s->loop, 0); return; }
                s->found = true;
            }, &s);
        if (op) { while (!s.done) pa_threaded_mainloop_wait(m_mainloop); pa_operation_unref(op); }
        pa_threaded_mainloop_unlock(m_mainloop);

        if (s.found) {
            qInfo(lcAudioPulse) << "resolveSourceName:" << candidate << "verified";
            return candidate;
        }
        if (requested.isEmpty()) return {};
        qWarning(lcAudioPulse) << "resolveSourceName: source" << candidate
                               << "not found in PA, falling back to default";
        return getDefaultSource();
    }

    // --- VOLUME CONTROL ---
    // Names passed here are pre-resolved by VolumeHandler — always valid.

    void PulseAudioHandler::setSinkVolume(const QString& sinkName, int volume) {
        const int safe = std::clamp(volume, 0, 255);

        // Prefer ALSA hardware volume — no soft-volume bit-depth penalty, and
        // survives a future PipeWire migration unchanged (snd_mixer_* talks to
        // the kernel ALSA control interface, bypassing any audio server).
        if (!m_alsaCardDevice.isEmpty()) {
            const QByteArray card = m_alsaCardDevice.toUtf8();
            snd_mixer_t* handle = nullptr;
            if (snd_mixer_open(&handle, 0) == 0) {
                auto cleanup = qScopeGuard([handle]{ snd_mixer_close(handle); });
                if (snd_mixer_attach(handle, card.constData()) == 0 &&
                    snd_mixer_selem_register(handle, nullptr, nullptr) == 0 &&
                    snd_mixer_load(handle) == 0) {
                    // Pick the element with the largest playback-volume range.
                    // On IQaudIO PCM5122 the mixer exposes 'Analogue' (0-1) before
                    // 'Digital' (0-207); using the first element would effectively
                    // give only two volume levels.  The widest range is the real DAC
                    // volume control.
                    snd_mixer_elem_t* best = nullptr;
                    long bestLo = 0, bestHi = 0, bestRange = 0;
                    for (auto* e = snd_mixer_first_elem(handle); e; e = snd_mixer_elem_next(e)) {
                        if (!snd_mixer_selem_has_playback_volume(e)) continue;
                        long lo = 0, hi = 0;
                        snd_mixer_selem_get_playback_volume_range(e, &lo, &hi);
                        if (hi - lo > bestRange) {
                            bestRange = hi - lo;
                            best = e; bestLo = lo; bestHi = hi;
                        }
                    }
                    if (best) {
                        const long alsaVol = bestLo + static_cast<long>((bestHi - bestLo) * safe / 255.0 + 0.5);
                        snd_mixer_selem_set_playback_volume_all(best, alsaVol);
                        qInfo(lcAudioPulse) << "setSinkVolume ALSA" << m_alsaCardDevice
                                           << "vol=" << safe << "alsa=" << alsaVol << "/" << bestHi;
                        return;
                    }
                }
            }
            qWarning(lcAudioPulse) << "setSinkVolume: ALSA mixer failed for" << m_alsaCardDevice
                                   << "— falling back to PA soft-volume";
        }

        // PA soft-volume fallback (non-ALSA sinks, or ALSA card not yet cached)
        if (!m_mainloop || !m_context || sinkName.isEmpty()) return;
        const pa_volume_t paVol = static_cast<pa_volume_t>((safe * PA_VOLUME_NORM) / 255.0);
        const QByteArray nb = sinkName.toUtf8();
        struct ChInfo { pa_threaded_mainloop* loop; uint8_t ch{2}; bool done{false}; };
        ChInfo info{m_mainloop};
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* qop = pa_context_get_sink_info_by_name(m_context, nb.constData(),
            [](pa_context*, const pa_sink_info* i, int eol, void* ud) {
                auto* s = static_cast<ChInfo*>(ud);
                if (eol > 0) { s->done = true; pa_threaded_mainloop_signal(s->loop, 0); return; }
                if (i) { s->ch = i->channel_map.channels; }
            }, &info);
        if (qop) { while (!info.done) pa_threaded_mainloop_wait(m_mainloop); pa_operation_unref(qop); }
        pa_cvolume cv;
        pa_cvolume_set(&cv, info.ch, paVol);
        qInfo(lcAudioPulse) << "setSinkVolume PA" << sinkName << "vol=" << safe << "pa=" << paVol;
        pa_operation* vop = pa_context_set_sink_volume_by_name(m_context, nb.constData(), &cv, nullptr, nullptr);
        if (vop) pa_operation_unref(vop);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSourceVolume(const QString& sourceName, int volume) {
        if (!m_mainloop || !m_context || sourceName.isEmpty()) return;
        const int safe = std::clamp(volume, 0, 255);
        const pa_volume_t paVol = static_cast<pa_volume_t>((safe * PA_VOLUME_NORM) / 255.0);
        const QByteArray nb = sourceName.toUtf8();

        struct ChInfo { pa_threaded_mainloop* loop; uint8_t ch{1}; bool done{false}; };
        ChInfo info{m_mainloop};
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* qop = pa_context_get_source_info_by_name(m_context, nb.constData(),
            [](pa_context*, const pa_source_info* i, int eol, void* ud) {
                auto* s = static_cast<ChInfo*>(ud);
                if (eol > 0 || !i) { s->done = true; pa_threaded_mainloop_signal(s->loop, 0); return; }
                s->ch = i->channel_map.channels;
                s->done = true; pa_threaded_mainloop_signal(s->loop, 0);
            }, &info);
        if (qop) { while (!info.done) pa_threaded_mainloop_wait(m_mainloop); pa_operation_unref(qop); }

        pa_cvolume cv;
        pa_cvolume_set(&cv, info.ch, paVol);
        qInfo(lcAudioPulse) << "setSourceVolume" << sourceName << "vol=" << safe
                            << "pa=" << paVol << "ch=" << info.ch;
        pa_operation* vop = pa_context_set_source_volume_by_name(m_context, nb.constData(), &cv, nullptr, nullptr);
        if (vop) pa_operation_unref(vop);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSinkMute(const QString& sinkName, bool mute) {
        // Prefer ALSA hardware mute switch — instant, no soft-volume interaction,
        // works identically under PulseAudio and PipeWire.
        if (!m_alsaCardDevice.isEmpty()) {
            const QByteArray card = m_alsaCardDevice.toUtf8();
            snd_mixer_t* handle = nullptr;
            if (snd_mixer_open(&handle, 0) == 0) {
                auto cleanup = qScopeGuard([handle]{ snd_mixer_close(handle); });
                if (snd_mixer_attach(handle, card.constData()) == 0 &&
                    snd_mixer_selem_register(handle, nullptr, nullptr) == 0 &&
                    snd_mixer_load(handle) == 0) {
                    // Prefer an element that has BOTH volume AND switch — on PCM5122 that
                    // is 'Digital'.  Pure-switch elements like 'Auto Mute' are chip feature
                    // flags, not actual audio muting.  Among volume+switch elements pick the
                    // one with the largest range; fall back to switch-only if none found.
                    snd_mixer_elem_t* muteElem = nullptr;
                    long bestRange = -1;
                    for (auto* e = snd_mixer_first_elem(handle); e; e = snd_mixer_elem_next(e)) {
                        if (!snd_mixer_selem_has_playback_switch(e)) continue;
                        if (snd_mixer_selem_has_playback_volume(e)) {
                            long lo = 0, hi = 0;
                            snd_mixer_selem_get_playback_volume_range(e, &lo, &hi);
                            if (hi - lo > bestRange) {
                                bestRange = hi - lo;
                                muteElem = e;
                            }
                        } else if (bestRange < 0) {
                            muteElem = e; // switch-only fallback
                        }
                    }
                    if (muteElem) {
                        snd_mixer_selem_set_playback_switch_all(muteElem, mute ? 0 : 1);
                        qInfo(lcAudioPulse) << "setSinkMute ALSA" << m_alsaCardDevice << mute;
                        return;
                    }
                }
            }
            qWarning(lcAudioPulse) << "setSinkMute: ALSA mixer failed for" << m_alsaCardDevice
                                   << "— falling back to PA mute";
        }

        // PA mute fallback
        if (!m_mainloop || !m_context || sinkName.isEmpty()) return;
        const QByteArray nb = sinkName.toUtf8();
        pa_threaded_mainloop_lock(m_mainloop);
        qInfo(lcAudioPulse) << "setSinkMute PA" << sinkName << mute;
        pa_operation* o = pa_context_set_sink_mute_by_name(m_context, nb.constData(), mute ? 1 : 0, nullptr, nullptr);
        if (o) pa_operation_unref(o);
        pa_threaded_mainloop_unlock(m_mainloop);
    }

    void PulseAudioHandler::setSourceMute(const QString& sourceName, bool mute) {
        if (!m_mainloop || !m_context || sourceName.isEmpty()) return;
        const QByteArray nb = sourceName.toUtf8();
        pa_threaded_mainloop_lock(m_mainloop);
        pa_operation* o = pa_context_set_source_mute_by_name(m_context, nb.constData(), mute ? 1 : 0, nullptr, nullptr);
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
            // Prefer the ALSA card name over the generic PulseAudio description.
            // On JourneyOS, udev-detect reports both the IQAudio DAC and the
            // onboard headphone output as "Built-in Audio Stereo" — the ALSA card
            // name ("IQaudIODAC", "bcm2835_alsa", etc.) is unambiguous.
            const char *cardName = pa_proplist_gets(i->proplist, "alsa.card_name");
            device.description = (cardName && *cardName)
                                     ? QString::fromUtf8(cardName)
                                     : QString::fromUtf8(i->description);
            device.value = QString::fromUtf8(i->name);
            // Store ALSA card number so getSinks() can remove duplicate PA sinks
            // that udev-detect creates for the same physical card.
            const char *cardNumStr = pa_proplist_gets(i->proplist, "alsa.card");
            device.card = cardNumStr ? std::atoi(cardNumStr) : -1;
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

        // Deduplicate: PulseAudio udev-detect creates two PA sinks for the same
        // physical ALSA card when the device tree gives it both a platform alias
        // ("platform-soc_sound") and an ALSA card name ("IQaudIODAC").  Both map
        // to the same hw:N device and would otherwise appear twice in the UI.
        // Strategy: group by ALSA card number; within each group keep the sink
        // whose PA name is NOT the platform-* path (the named path is more stable
        // and matches what "hw:IQaudIODAC" would select explicitly).
        QMap<int, int> preferredIdx; // ALSA card# → index in state.devices
        for (int i = 0; i < state.devices.size(); ++i) {
            const int cardNum = state.devices[i].card;
            if (cardNum < 0) continue; // not an ALSA sink — keep unconditionally
            const bool isPlatform = state.devices[i].value.contains(QLatin1String("platform-"));
            if (!preferredIdx.contains(cardNum)) {
                preferredIdx[cardNum] = i;
            } else {
                // Prefer the non-platform entry
                const bool existingIsPlatform = state.devices[preferredIdx[cardNum]].value.contains(QLatin1String("platform-"));
                if (existingIsPlatform && !isPlatform)
                    preferredIdx[cardNum] = i;
            }
        }

        EngineDeviceList result;
        for (int i = 0; i < state.devices.size(); ++i) {
            const int cardNum = state.devices[i].card;
            if (cardNum < 0 || preferredIdx.value(cardNum, i) == i)
                result.append(state.devices[i]);
        }
        return result;
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

        // NOTE: on JourneyOS the only available sources are monitor pseudo-sources
        // (alsa_output.*.monitor) — there is no physical microphone.  The HDMI
        // monitor source is used as a loopback input workaround for Android Auto.
        // Do NOT filter out .monitor sources here.

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