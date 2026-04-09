// DESIGN STUDIO MOCK — NOT used in the production build.
// The real AndroidAutoConnectivityState is a C++ Q_ENUM registered as a
// context property in main.cpp. This QML stub exists solely so Qt Design
// Studio can resolve the type and preview the UI without a C++ backend.
pragma Singleton
import QtQuick

QtObject {
    readonly property int AA_DISCONNECTED: 0
    readonly property int AA_CONNECTING:   1
    readonly property int AA_CONNECTED:    2
}
