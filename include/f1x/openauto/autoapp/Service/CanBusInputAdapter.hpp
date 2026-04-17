#pragma once
#include <QObject>

// Forward declarations — keeps this header free of heavy includes.
namespace JourneyOS { class CanBusReceiver; }
namespace f1x::openauto::autoapp::UI::ViewModel { class VolumeViewModel; }

namespace f1x::openauto::autoapp::service {

// CanBusInputAdapter
//
// Bridges CanBusReceiver steering-wheel button signals into OpenAuto:
//
//   volumeUp / volumeDown  → VolumeViewModel::setVolumeSink (±step)
//   muteToggle             → VolumeViewModel::setVolumeSinkMute (toggle)
//   handsFreeToggle        → Qt::Key_P (KEYCODE_CALL) injected into app
//   navLeft / navRight     → Qt::Key_Left / Key_Right
//   scrollUp / scrollDown  → Qt::Key_Up / Key_Down
//   select                 → Qt::Key_Return
//
// Key events are posted to the application focus window so that InputDevice
// (installed as a global event filter on QApplication) forwards them to the
// active Android Auto session via InputSourceService.
//
// Volume signals also indirectly open the volume popup: Journey.qml listens
// to volumePopupHandler.volumeSinkChanged and opens the slider overlay.

class CanBusInputAdapter : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(CanBusInputAdapter)

public:
    // volumeStep: how much to increment/decrement the sink volume (0-255 scale).
    explicit CanBusInputAdapter(
        UI::ViewModel::VolumeViewModel& volumeHandler,
        int volumeStep = 10,
        QObject* parent = nullptr);

    // Connect all button signals from receiver to this adapter.
    // Safe to call once after receiver is constructed.
    void connectReceiver(JourneyOS::CanBusReceiver* receiver);

private slots:
    void onVolumeUp();
    void onVolumeDown();
    void onMuteToggle();
    void onHandsFreeToggle();
    void onNavLeft();
    void onNavRight();
    void onScrollUp();
    void onScrollDown();
    void onSelect();

private:
    // Post a synthetic key press + release to the active focus window.
    // InputDevice's global event filter on QApplication will intercept it
    // and translate it to an AA ButtonEvent.
    static void postKey(int qtKey);

    UI::ViewModel::VolumeViewModel& volumeHandler_;
    int volumeStep_;
};

} // namespace f1x::openauto::autoapp::service