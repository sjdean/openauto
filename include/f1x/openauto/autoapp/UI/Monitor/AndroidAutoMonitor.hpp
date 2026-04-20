#ifndef OPENAUTO_ANDROIDAUTOMONITOR_HPP
#define OPENAUTO_ANDROIDAUTOMONITOR_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <f1x/openauto/autoapp/UI/Monitor/IAndroidAutoMonitor.hpp>

#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoAudioFocusState.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

  class AndroidAutoMonitor : public QObject, IAndroidAutoMonitor {
    Q_OBJECT
    Q_PROPERTY(common::Enum::AndroidAutoConnectivityState::Value state READ getState WRITE setState NOTIFY connectivityStateChanged)
    Q_PROPERTY(common::Enum::AndroidAutoConnectivityMethod::Value method READ getMethod WRITE setMethod NOTIFY connectivityMethodChanged)
    Q_PROPERTY(common::Enum::AndroidAutoAudioFocusState::Value audioFocusState READ getAudioFocusState NOTIFY audioFocusStateChanged)
  public:
    explicit AndroidAutoMonitor(QObject *parent = nullptr);
    void onConnectionMethodUpdate(common::Enum::AndroidAutoConnectivityMethod::Value method);
    void onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::Value state);

    common::Enum::AndroidAutoConnectivityState::Value getState() const;
    common::Enum::AndroidAutoConnectivityMethod::Value getMethod() const;
    common::Enum::AndroidAutoAudioFocusState::Value getAudioFocusState() const;

    void setState(common::Enum::AndroidAutoConnectivityState::Value value);
    void setMethod(common::Enum::AndroidAutoConnectivityMethod::Value value);

    /**
     * Called from the Android Auto session (IO thread) when an AudioFocusRequest
     * arrives on the Control channel. Signals QML and any HU media players.
     *
     * Focus states and their meaning for HU-local media (MP3, Radio):
     *   Gained       — AA owns audio exclusively; pause HU media.
     *   GainedTransient — AA owns audio briefly; pause, expect resume soon.
     *   GainedCanDuck  — AA wants audio; duck HU media ~40% (navigation guidance).
     *   Released     — AA released focus; resume / restore HU media volume.
     *
     * TODO(audio-focus): When HU media players exist (5.1), connect
     * audioFocusStateChanged to each player's duck/pause/resume slots here.
     */
    void onAudioFocusChanged(common::Enum::AndroidAutoAudioFocusState::Value state);

    /**
     * Called from the Android Auto session (IO thread) when the first PingRequest
     * from the phone is received. Emits phoneTimestampReceived() queued to the main
     * thread so that TimeController can consume it safely.
     */
    void notifyPhoneTimestamp(quint64 epochMicroseconds);

  signals:
    void connectivityStateChanged(common::Enum::AndroidAutoConnectivityState::Value state);
    void connectivityMethodChanged(common::Enum::AndroidAutoConnectivityMethod::Value method);
    void audioFocusStateChanged(common::Enum::AndroidAutoAudioFocusState::Value state);
    void phoneTimestampReceived(quint64 epochMicroseconds);

  private:
    common::Enum::AndroidAutoConnectivityMethod::Value m_connectivityMethod;
    common::Enum::AndroidAutoConnectivityState::Value m_connectivityState;
    common::Enum::AndroidAutoAudioFocusState::Value m_audioFocusState;

  };
}
#endif //OPENAUTO_ANDROIDAUTOMONITOR_HPP