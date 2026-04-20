#pragma once

#include <QObject>

namespace f1x::openauto::common::Enum {

  class AndroidAutoAudioFocusState : public QObject {
    Q_OBJECT

  public:
    enum Value {
      // No active focus request — HU media plays normally.
      Idle,
      // AA has exclusive audio focus. HU media should pause entirely.
      // Sent for: music playback, phone calls, any stream that owns audio.
      Gained,
      // AA has transient exclusive focus (brief, will release shortly).
      // HU media should pause; expected to resume when Released arrives.
      GainedTransient,
      // AA wants audio but the HU may duck instead of pausing.
      // HU media should lower volume ~40%; normal for navigation guidance.
      GainedCanDuck,
      // AA has released focus. HU media should resume / restore volume.
      Released,
    };

    Q_ENUM(Value)
  };

} // namespace f1x::openauto::common::Enum
