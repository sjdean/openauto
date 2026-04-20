#pragma once
#include <QtCore/qglobal.h>
#ifdef Q_OS_LINUX

#include "IAudioHandler.h"
#include <QString>

#include <pipewire/pipewire.h>
#include <pipewire/extensions/metadata.h>

namespace f1x::openauto::autoapp::UI::Backend::Audio {

class PipeWireAudioHandler : public IAudioHandler {
public:
    explicit PipeWireAudioHandler();
    ~PipeWireAudioHandler() override;

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
    struct NodeInfo {
        uint32_t id;
        QString  name;         // PW_KEY_NODE_NAME
        QString  description;  // PW_KEY_NODE_DESCRIPTION
        bool     isSink;
    };

    pw_thread_loop *m_loop     = nullptr;
    pw_context     *m_context  = nullptr;
    pw_core        *m_core     = nullptr;
    pw_registry    *m_registry = nullptr;
    pw_metadata    *m_metadata = nullptr;

    spa_hook m_coreListener;
    spa_hook m_registryListener;
    spa_hook m_metadataListener;

    QList<NodeInfo> m_nodes;
    QString         m_defaultSink;
    QString         m_defaultSource;
    int             m_pendingSeq = -1;

    // Static C-compatible callbacks
    static void onGlobal(void *data, uint32_t id, uint32_t permissions,
                         const char *type, uint32_t version,
                         const struct spa_dict *props);
    static void onGlobalRemove(void *data, uint32_t id);
    static void onCoreDone(void *data, uint32_t id, int seq);
    static int  onMetadataProperty(void *data, uint32_t subject, const char *key,
                                   const char *type, const char *value);

    // Helpers — must be called with loop locked
    uint32_t resolveNodeId(const QString& name, bool isSink) const;
    void     setNodeVolume(uint32_t nodeId, float linear);
    void     setNodeMute(uint32_t nodeId, bool mute);
};

} // namespace f1x::openauto::autoapp::UI::Backend::Audio
#endif