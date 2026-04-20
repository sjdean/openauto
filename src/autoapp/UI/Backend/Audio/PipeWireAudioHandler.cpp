#include "f1x/openauto/autoapp/UI/Backend/Audio/PipeWireAudioHandler.hpp"
#ifdef Q_OS_LINUX

#include <spa/param/props.h>
#include <spa/pod/builder.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <qloggingcategory.h>

#include <algorithm>
#include <cstring>

Q_LOGGING_CATEGORY(lcAudioPW, "journeyos.audio.pipewire")

namespace f1x::openauto::autoapp::UI::Backend::Audio {

// ─── Static callback tables ──────────────────────────────────────────────────

static const pw_registry_events s_registryEvents = {
    .version       = PW_VERSION_REGISTRY_EVENTS,
    .global        = PipeWireAudioHandler::onGlobal,
    .global_remove = PipeWireAudioHandler::onGlobalRemove,
};

static const pw_core_events s_coreEvents = {
    .version = PW_VERSION_CORE_EVENTS,
    .done    = PipeWireAudioHandler::onCoreDone,
};

static const pw_metadata_events s_metadataEvents = {
    .version  = PW_VERSION_METADATA_EVENTS,
    .property = PipeWireAudioHandler::onMetadataProperty,
};

// ─── Helper: parse the default device name from PipeWire metadata value ──────
// PipeWire >= 0.3.33 encodes the value as JSON: {"name":"<node-name>","volumes":[...]}
// Older builds may use a bare name string. Handle both.

static QString parseDefaultName(const char *value) {
    if (!value || value[0] == '\0') return {};
    const auto doc = QJsonDocument::fromJson(QByteArray(value));
    if (!doc.isNull() && doc.isObject()) {
        const QString name = doc.object().value(QStringLiteral("name")).toString();
        if (!name.isEmpty()) return name;
    }
    return QString::fromUtf8(value);
}

// ─── Constructor ─────────────────────────────────────────────────────────────

PipeWireAudioHandler::PipeWireAudioHandler() {
    pw_init(nullptr, nullptr);

    m_loop = pw_thread_loop_new("journeyos-pw", nullptr);
    if (!m_loop) {
        qCritical(lcAudioPW) << "pw_thread_loop_new failed";
        return;
    }

    m_context = pw_context_new(pw_thread_loop_get_loop(m_loop), nullptr, 0);
    if (!m_context) {
        qCritical(lcAudioPW) << "pw_context_new failed";
        return;
    }

    // Lock before starting so we own the mutex when the thread first tries to iterate.
    pw_thread_loop_lock(m_loop);

    m_core = pw_context_connect(m_context, nullptr, 0);
    if (!m_core) {
        qCritical(lcAudioPW) << "pw_context_connect failed — PipeWire daemon not running?";
        pw_thread_loop_unlock(m_loop);
        return;
    }

    pw_core_add_listener(m_core, &m_coreListener, &s_coreEvents, this);

    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(m_registry, &m_registryListener, &s_registryEvents, this);

    // Start the thread — it will block on the lock until pw_thread_loop_wait releases it.
    pw_thread_loop_start(m_loop);

    // Request a roundtrip: when the done callback fires with this seq, all registry
    // globals (and metadata property callbacks) that were pending have been delivered.
    m_pendingSeq = pw_core_sync(m_core, PW_ID_CORE, 0);
    pw_thread_loop_wait(m_loop);  // releases lock, waits, reacquires

    pw_thread_loop_unlock(m_loop);

    const auto nSinks   = std::count_if(m_nodes.begin(), m_nodes.end(), [](const NodeInfo& n){ return  n.isSink; });
    const auto nSources = std::count_if(m_nodes.begin(), m_nodes.end(), [](const NodeInfo& n){ return !n.isSink; });
    qInfo(lcAudioPW) << "connected — sinks:" << nSinks << "sources:" << nSources
                     << "| default sink:"   << m_defaultSink
                     << "| default source:" << m_defaultSource;
}

// ─── Destructor ──────────────────────────────────────────────────────────────

PipeWireAudioHandler::~PipeWireAudioHandler() {
    if (m_loop) {
        pw_thread_loop_lock(m_loop);

        if (m_metadata) {
            pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_metadata));
            m_metadata = nullptr;
        }
        if (m_registry) {
            pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_registry));
            m_registry = nullptr;
        }
        if (m_core) {
            pw_core_disconnect(m_core);
            m_core = nullptr;
        }

        pw_thread_loop_unlock(m_loop);
        pw_thread_loop_stop(m_loop);
        pw_thread_loop_destroy(m_loop);
        m_loop = nullptr;
    }

    if (m_context) {
        pw_context_destroy(m_context);
        m_context = nullptr;
    }

    pw_deinit();
}

// ─── Registry callbacks ──────────────────────────────────────────────────────

void PipeWireAudioHandler::onGlobal(void *data, uint32_t id, uint32_t /*permissions*/,
                                     const char *type, uint32_t /*version*/,
                                     const struct spa_dict *props) {
    auto *self = static_cast<PipeWireAudioHandler*>(data);

    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        const char *mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        if (!mediaClass) return;

        const bool isSink   = (strcmp(mediaClass, "Audio/Sink")   == 0 ||
                                strcmp(mediaClass, "Audio/Duplex") == 0);
        const bool isSource = (strcmp(mediaClass, "Audio/Source") == 0 ||
                                strcmp(mediaClass, "Audio/Duplex") == 0);
        if (!isSink && !isSource) return;

        const char *name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char *desc = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
        if (!name) name = "";

        NodeInfo info;
        info.id          = id;
        info.name        = QString::fromUtf8(name);
        info.description = desc ? QString::fromUtf8(desc) : info.name;
        info.isSink      = isSink;
        self->m_nodes.append(info);

        // Duplex nodes serve as both sink and source; add a second entry as source.
        if (isSink && isSource) {
            NodeInfo src  = info;
            src.isSink    = false;
            self->m_nodes.append(src);
        }

    } else if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
        const char *metaName = spa_dict_lookup(props, PW_KEY_METADATA_NAME);
        if (!metaName || strcmp(metaName, "default") != 0) return;

        self->m_metadata = reinterpret_cast<pw_metadata*>(
            pw_registry_bind(self->m_registry, id,
                             PW_TYPE_INTERFACE_Metadata, PW_VERSION_METADATA, 0));
        if (self->m_metadata)
            pw_metadata_add_listener(self->m_metadata, &self->m_metadataListener,
                                     &s_metadataEvents, self);
    }
}

void PipeWireAudioHandler::onGlobalRemove(void *data, uint32_t id) {
    auto *self = static_cast<PipeWireAudioHandler*>(data);
    self->m_nodes.erase(
        std::remove_if(self->m_nodes.begin(), self->m_nodes.end(),
                       [id](const NodeInfo& n){ return n.id == id; }),
        self->m_nodes.end());
}

// ─── Core done callback ──────────────────────────────────────────────────────

void PipeWireAudioHandler::onCoreDone(void *data, uint32_t id, int seq) {
    auto *self = static_cast<PipeWireAudioHandler*>(data);
    if (id == PW_ID_CORE && seq == self->m_pendingSeq) {
        self->m_pendingSeq = -1;
        pw_thread_loop_signal(self->m_loop, false);
    }
}

// ─── Metadata callback ───────────────────────────────────────────────────────

int PipeWireAudioHandler::onMetadataProperty(void *data, uint32_t /*subject*/,
                                              const char *key, const char * /*type*/,
                                              const char *value) {
    if (!key) return 0;
    auto *self = static_cast<PipeWireAudioHandler*>(data);

    if (strcmp(key, "default.audio.sink") == 0) {
        self->m_defaultSink = parseDefaultName(value);
        qDebug(lcAudioPW) << "default sink ->" << self->m_defaultSink;
    } else if (strcmp(key, "default.audio.source") == 0) {
        self->m_defaultSource = parseDefaultName(value);
        qDebug(lcAudioPW) << "default source ->" << self->m_defaultSource;
    }
    return 0;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

uint32_t PipeWireAudioHandler::resolveNodeId(const QString& name, bool isSink) const {
    const QString target = name.isEmpty()
        ? (isSink ? m_defaultSink : m_defaultSource)
        : name;

    for (const auto& n : m_nodes) {
        if (n.isSink == isSink && n.name == target)
            return n.id;
    }
    if (!target.isEmpty())
        qWarning(lcAudioPW) << "node not found:" << target;
    return SPA_ID_INVALID;
}

void PipeWireAudioHandler::setNodeVolume(uint32_t nodeId, float linear) {
    if (nodeId == SPA_ID_INVALID || !m_registry) return;

    auto *proxy = reinterpret_cast<pw_node*>(
        pw_registry_bind(m_registry, nodeId, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0));
    if (!proxy) return;

    uint8_t buf[256];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buf, sizeof(buf));
    const float vol = std::clamp(linear, 0.0f, 1.0f);
    struct spa_pod *pod = static_cast<struct spa_pod*>(
        spa_pod_builder_add_object(&b,
            SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
            SPA_PROP_volume, SPA_POD_Float(vol)));

    pw_node_set_param(proxy, SPA_PARAM_Props, 0, pod);
    pw_proxy_destroy(reinterpret_cast<pw_proxy*>(proxy));
}

void PipeWireAudioHandler::setNodeMute(uint32_t nodeId, bool mute) {
    if (nodeId == SPA_ID_INVALID || !m_registry) return;

    auto *proxy = reinterpret_cast<pw_node*>(
        pw_registry_bind(m_registry, nodeId, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0));
    if (!proxy) return;

    uint8_t buf[256];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buf, sizeof(buf));
    struct spa_pod *pod = static_cast<struct spa_pod*>(
        spa_pod_builder_add_object(&b,
            SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
            SPA_PROP_mute, SPA_POD_Bool(mute)));

    pw_node_set_param(proxy, SPA_PARAM_Props, 0, pod);
    pw_proxy_destroy(reinterpret_cast<pw_proxy*>(proxy));
}

// ─── IAudioHandler implementation ────────────────────────────────────────────

QString PipeWireAudioHandler::getDefaultSink() {
    if (!m_loop) return {};
    pw_thread_loop_lock(m_loop);
    const QString result = m_defaultSink;
    pw_thread_loop_unlock(m_loop);
    return result;
}

QString PipeWireAudioHandler::getDefaultSource() {
    if (!m_loop) return {};
    pw_thread_loop_lock(m_loop);
    const QString result = m_defaultSource;
    pw_thread_loop_unlock(m_loop);
    return result;
}

void PipeWireAudioHandler::setSinkVolume(const QString& deviceName, int volume) {
    if (!m_loop) return;
    const float linear = std::clamp(volume, 0, 255) / 255.0f;
    pw_thread_loop_lock(m_loop);
    setNodeVolume(resolveNodeId(deviceName, true), linear);
    pw_thread_loop_unlock(m_loop);
}

void PipeWireAudioHandler::setSourceVolume(const QString& deviceName, int volume) {
    if (!m_loop) return;
    const float linear = std::clamp(volume, 0, 255) / 255.0f;
    pw_thread_loop_lock(m_loop);
    setNodeVolume(resolveNodeId(deviceName, false), linear);
    pw_thread_loop_unlock(m_loop);
}

void PipeWireAudioHandler::setSinkMute(const QString& deviceName, bool mute) {
    if (!m_loop) return;
    pw_thread_loop_lock(m_loop);
    setNodeMute(resolveNodeId(deviceName, true), mute);
    pw_thread_loop_unlock(m_loop);
}

void PipeWireAudioHandler::setSourceMute(const QString& deviceName, bool mute) {
    if (!m_loop) return;
    pw_thread_loop_lock(m_loop);
    setNodeMute(resolveNodeId(deviceName, false), mute);
    pw_thread_loop_unlock(m_loop);
}

EngineDeviceList PipeWireAudioHandler::getSinks() {
    if (!m_loop) return {};
    pw_thread_loop_lock(m_loop);
    EngineDeviceList result;
    for (const auto& n : m_nodes) {
        if (!n.isSink) continue;
        EngineDevice dev;
        dev.description = n.description;
        dev.value       = n.name;
        dev.iconname    = dev.GuessIconName();
        result.append(dev);
    }
    pw_thread_loop_unlock(m_loop);
    return result;
}

EngineDeviceList PipeWireAudioHandler::getSources() {
    if (!m_loop) return {};
    pw_thread_loop_lock(m_loop);
    EngineDeviceList result;
    for (const auto& n : m_nodes) {
        if (n.isSink) continue;
        EngineDevice dev;
        dev.description = n.description;
        dev.value       = n.name;
        result.append(dev);
    }
    pw_thread_loop_unlock(m_loop);
    return result;
}

std::vector<std::pair<std::string, std::string>> PipeWireAudioHandler::getDeviceList() {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& dev : getSinks())
        result.push_back({ dev.value.toStdString(), dev.description.toStdString() });
    return result;
}

} // namespace f1x::openauto::autoapp::UI::Backend::Audio
#endif