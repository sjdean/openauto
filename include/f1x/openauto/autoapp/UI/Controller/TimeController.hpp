#pragma once

#include <QDateTime>
#include <QObject>

namespace f1x::openauto::autoapp::UI::Controller {

/**
 * Manages system time synchronisation from multiple sources.
 *
 * Sources are ranked by priority (highest wins):
 *   NTP (3) > RTC (2) > AndroidAuto (1) > Unknown (0)
 *
 * Once a source has been used to set the time it will not be accepted again
 * in the same process lifetime, preventing unnecessary repeated set calls.
 * A higher-priority source can always supersede a lower one.
 *
 * On Linux, time is applied via org.freedesktop.timedate1 SetTime D-Bus call.
 * On other platforms the apply step is a no-op (for development builds).
 */
class TimeController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isTimeSet READ isTimeSet NOTIFY timeChanged)
    Q_PROPERTY(TimeSource timeSource READ timeSource NOTIFY timeChanged)
    Q_PROPERTY(QString lastSyncedAt READ lastSyncedAt NOTIFY timeChanged)
    Q_PROPERTY(QString timeSourceName READ timeSourceName NOTIFY timeChanged)

public:
    enum class TimeSource {
        Unknown     = 0,
        AndroidAuto = 1,
        RTC         = 2,
        NTP         = 3
    };
    Q_ENUM(TimeSource)

    explicit TimeController(QObject *parent = nullptr);

    bool        isTimeSet()      const;
    TimeSource  timeSource()     const;
    QString     lastSyncedAt()   const;
    QString     timeSourceName() const;

public slots:
    /**
     * Generic entry point. Offer a candidate time from a named source.
     * The offer is accepted only if:
     *  - the source priority is strictly higher than the current source, AND
     *  - this source has not already been used in this session, AND
     *  - the candidate time passes the sanity check (year 2024–2035).
     */
    void offerTime(quint64 epochMicroseconds, TimeSource source);

    /**
     * Convenience slot — equivalent to offerTime(ts, TimeSource::AndroidAuto).
     * Connected to AndroidAutoMonitor::phoneTimestampReceived in main.cpp.
     */
    void offerTimeFromAndroidAuto(quint64 epochMicroseconds);

signals:
    void timeChanged();

private:
    void checkStartupTimeSources();
    bool applySystemTime(const QDateTime &dt);

    static int     sourcePriority(TimeSource source);
    static bool    sanityCheck(const QDateTime &dt);
    static QString sourceToString(TimeSource source);

    bool       m_isTimeSet{false};
    TimeSource m_timeSource{TimeSource::Unknown};
    QDateTime  m_lastSyncedAt;
    // Per-source "already used" flags, indexed by static_cast<int>(TimeSource)
    bool       m_usedSources[4]{false, false, false, false};
};

} // namespace f1x::openauto::autoapp::UI::Controller