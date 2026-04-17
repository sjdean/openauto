#include <f1x/openauto/autoapp/Service/CanBusInputAdapter.hpp>
#include <f1x/openauto/autoapp/UI/ViewModel/VolumeViewModel.hpp>

#ifdef JOURNEYOS_CANBUS_RECEIVER
#include <JourneyOS/CanBus/CanBusReceiver.h>
#endif

#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>
#include <algorithm>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcCanBusInput, "journeyos.canbus.input")

namespace f1x::openauto::autoapp::service {

CanBusInputAdapter::CanBusInputAdapter(UI::ViewModel::VolumeViewModel& volumeHandler,
                                       int volumeStep,
                                       QObject* parent)
    : QObject(parent)
    , volumeHandler_(volumeHandler)
    , volumeStep_(volumeStep)
{
}

void CanBusInputAdapter::connectReceiver(JourneyOS::CanBusReceiver* receiver)
{
#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (!receiver) return;

    connect(receiver, &JourneyOS::CanBusReceiver::volumeUp,        this, &CanBusInputAdapter::onVolumeUp,        Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::volumeDown,      this, &CanBusInputAdapter::onVolumeDown,      Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::muteToggle,      this, &CanBusInputAdapter::onMuteToggle,      Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::handsFreeToggle, this, &CanBusInputAdapter::onHandsFreeToggle, Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::navLeft,         this, &CanBusInputAdapter::onNavLeft,         Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::navRight,        this, &CanBusInputAdapter::onNavRight,        Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::scrollUp,        this, &CanBusInputAdapter::onScrollUp,        Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::scrollDown,      this, &CanBusInputAdapter::onScrollDown,      Qt::QueuedConnection);
    connect(receiver, &JourneyOS::CanBusReceiver::select,          this, &CanBusInputAdapter::onSelect,          Qt::QueuedConnection);

    qInfo(lcCanBusInput) << "CanBusInputAdapter: connected to receiver";
#else
    Q_UNUSED(receiver)
#endif
}

// ─── Volume ───────────────────────────────────────────────────────────────────

void CanBusInputAdapter::onVolumeUp()
{
    const int next = std::clamp(volumeHandler_.getVolumeSink() + volumeStep_,
                                volumeHandler_.getVolumeSinkMin(),
                                volumeHandler_.getVolumeSinkMax());
    qDebug(lcCanBusInput) << "volume up →" << next;
    volumeHandler_.setVolumeSink(next);
}

void CanBusInputAdapter::onVolumeDown()
{
    const int next = std::clamp(volumeHandler_.getVolumeSink() - volumeStep_,
                                volumeHandler_.getVolumeSinkMin(),
                                volumeHandler_.getVolumeSinkMax());
    qDebug(lcCanBusInput) << "volume down →" << next;
    volumeHandler_.setVolumeSink(next);
}

void CanBusInputAdapter::onMuteToggle()
{
    const bool nowMuted = !volumeHandler_.getVolumeSinkMute();
    qDebug(lcCanBusInput) << "mute toggle →" << nowMuted;
    volumeHandler_.setVolumeSinkMute(nowMuted);
}

// ─── Navigation / media keys ─────────────────────────────────────────────────

void CanBusInputAdapter::onHandsFreeToggle() { postKey(Qt::Key_P); }
void CanBusInputAdapter::onNavLeft()         { postKey(Qt::Key_Left); }
void CanBusInputAdapter::onNavRight()        { postKey(Qt::Key_Right); }
void CanBusInputAdapter::onScrollUp()        { postKey(Qt::Key_Up); }
void CanBusInputAdapter::onScrollDown()      { postKey(Qt::Key_Down); }
void CanBusInputAdapter::onSelect()          { postKey(Qt::Key_Return); }

// ─── Key injection ────────────────────────────────────────────────────────────

void CanBusInputAdapter::postKey(int qtKey)
{
    // InputDevice installs itself as a global event filter on QApplication,
    // which means it intercepts ALL events sent to ANY object in the app.
    // Posting press+release to the focus window is the cleanest target.
    QWindow* target = QGuiApplication::focusWindow();
    if (!target) {
        // Fall back to the first top-level window
        const auto windows = QGuiApplication::topLevelWindows();
        if (!windows.isEmpty())
            target = windows.first();
    }
    if (!target) {
        qWarning(lcCanBusInput) << "postKey: no window to deliver key" << qtKey;
        return;
    }

    qDebug(lcCanBusInput) << "postKey:" << qtKey << "→" << target->objectName();
    QCoreApplication::postEvent(target, new QKeyEvent(QEvent::KeyPress,   qtKey, Qt::NoModifier));
    QCoreApplication::postEvent(target, new QKeyEvent(QEvent::KeyRelease, qtKey, Qt::NoModifier));
}

} // namespace f1x::openauto::autoapp::service