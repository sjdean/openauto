// DESIGN STUDIO MOCK — NOT used in the production build.
// The real BluetoothConnectionStatus is a C++ Q_ENUM registered as a
// context property in main.cpp. This QML stub exists solely so Qt Design
// Studio can resolve the type and preview the UI without a C++ backend.
pragma Singleton
import QtQuick

QtObject {
    readonly property int BC_NOT_CONFIGURED: 0
    readonly property int BC_DISCONNECTED:   1
    readonly property int BC_CONNECTING:     2
    readonly property int BC_CONNECTED:      3
}
