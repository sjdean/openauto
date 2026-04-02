#include "f1x/openauto/autoapp/UI/Controller/TimeController.hpp"

#include <QDateTime>
#include <qloggingcategory.h>

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#endif

Q_LOGGING_CATEGORY(lcTime, "journeyos.time")

namespace f1x::openauto::autoapp::UI::Controller {

static constexpr int kMinSaneYear = 2024;
static constexpr int kMaxSaneYear = 2035;

TimeController::TimeController(QObject *parent) : QObject(parent) {
    checkStartupTimeSources();
}

bool TimeController::isTimeSet() const      { return m_isTimeSet; }
TimeController::TimeSource TimeController::timeSource() const { return m_timeSource; }

QString TimeController::lastSyncedAt() const {
    if (!m_isTimeSet) return QStringLiteral("never");
    return m_lastSyncedAt.toString(Qt::ISODate);
}

QString TimeController::timeSourceName() const {
    return sourceToString(m_timeSource);
}

void TimeController::checkStartupTimeSources() {
#ifdef Q_OS_LINUX
    QDBusInterface iface(
        QStringLiteral("org.freedesktop.timedate1"),
        QStringLiteral("/org/freedesktop/timedate1"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QDBusConnection::systemBus()
    );

    if (iface.isValid()) {
        const QDBusReply<QDBusVariant> reply = iface.call(
            QStringLiteral("Get"),
            QStringLiteral("org.freedesktop.timedate1"),
            QStringLiteral("NTPSynchronized")
        );
        if (reply.isValid() && reply.value().variant().toBool()) {
            qInfo(lcTime) << "startup: NTP is synchronized — time source established";
            m_isTimeSet  = true;
            m_timeSource = TimeSource::NTP;
            m_lastSyncedAt = QDateTime::currentDateTimeUtc();
            m_usedSources[static_cast<int>(TimeSource::NTP)] = true;
            emit timeChanged();
            return;
        }
    }
#endif

    // No NTP sync confirmed. Log the state of the system clock for diagnostics
    // but leave source as Unknown so a higher-quality source (RTC, AA) can set it.
    const int year = QDateTime::currentDateTimeUtc().date().year();
    if (year >= kMinSaneYear) {
        qInfo(lcTime) << "startup: NTP not active; system clock year=" << year
                      << " — will accept time from RTC or Android Auto";
    } else {
        qWarning(lcTime) << "startup: system clock looks bogus (year=" << year
                         << ") — awaiting time from RTC or Android Auto";
    }
}

void TimeController::offerTimeFromAndroidAuto(quint64 epochMicroseconds) {
    offerTime(epochMicroseconds, TimeSource::AndroidAuto);
}

void TimeController::offerTime(quint64 epochMicroseconds, TimeSource source) {
    // Reject if the current source already has equal-or-higher priority
    if (sourcePriority(source) <= sourcePriority(m_timeSource)) {
        qDebug(lcTime) << "offer from" << sourceToString(source)
                       << "rejected — current source" << sourceToString(m_timeSource)
                       << "has equal or higher priority";
        return;
    }

    // Each source is used at most once per process lifetime
    const int idx = static_cast<int>(source);
    if (m_usedSources[idx]) {
        qDebug(lcTime) << "offer from" << sourceToString(source)
                       << "ignored — already accepted once this session";
        return;
    }

    const QDateTime dt = QDateTime::fromMSecsSinceEpoch(
        static_cast<qint64>(epochMicroseconds / 1000), Qt::UTC);

    if (!sanityCheck(dt)) {
        qWarning(lcTime) << "offer from" << sourceToString(source)
                         << "failed sanity check — proposed date=" << dt.toString(Qt::ISODate);
        return;
    }

    qInfo(lcTime) << "accepting time" << dt.toString(Qt::ISODate)
                  << "from" << sourceToString(source);

    if (!applySystemTime(dt)) {
        qWarning(lcTime) << "failed to apply system time from" << sourceToString(source);
        return;
    }

    m_usedSources[idx] = true;
    m_isTimeSet  = true;
    m_timeSource = source;
    m_lastSyncedAt = QDateTime::currentDateTimeUtc();
    emit timeChanged();
}

bool TimeController::applySystemTime(const QDateTime &dt) {
#ifdef Q_OS_LINUX
    QDBusInterface iface(
        QStringLiteral("org.freedesktop.timedate1"),
        QStringLiteral("/org/freedesktop/timedate1"),
        QStringLiteral("org.freedesktop.timedate1"),
        QDBusConnection::systemBus()
    );

    if (!iface.isValid()) {
        qCritical(lcTime) << "org.freedesktop.timedate1 is not available";
        return false;
    }

    // SetTime(Int64 usec_utc, Boolean relative, Boolean interactive)
    const qlonglong usec = static_cast<qlonglong>(dt.toMSecsSinceEpoch()) * 1000LL;
    const QDBusReply<void> reply = iface.call(
        QStringLiteral("SetTime"),
        QVariant::fromValue(usec),
        false,   // absolute (not relative offset)
        false    // non-interactive — no polkit prompt
    );

    if (!reply.isValid()) {
        qCritical(lcTime) << "SetTime D-Bus call failed:" << reply.error().message();
        return false;
    }
    return true;
#else
    Q_UNUSED(dt)
    qInfo(lcTime) << "applySystemTime: no-op on non-Linux platform";
    return true;
#endif
}

int TimeController::sourcePriority(TimeSource source) {
    switch (source) {
        case TimeSource::NTP:         return 3;
        case TimeSource::RTC:         return 2;
        case TimeSource::AndroidAuto: return 1;
        case TimeSource::Unknown:     return 0;
    }
    return 0;
}

bool TimeController::sanityCheck(const QDateTime &dt) {
    if (!dt.isValid()) return false;
    const int year = dt.date().year();
    return year >= kMinSaneYear && year <= kMaxSaneYear;
}

QString TimeController::sourceToString(TimeSource source) {
    switch (source) {
        case TimeSource::NTP:         return QStringLiteral("NTP");
        case TimeSource::RTC:         return QStringLiteral("RTC");
        case TimeSource::AndroidAuto: return QStringLiteral("Android Auto");
        case TimeSource::Unknown:     return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

} // namespace f1x::openauto::autoapp::UI::Controller