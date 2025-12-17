import QtQuick
import QtQuick.Controls
import JourneyOS 1.0
import AndroidAutoMonitor
Window {
    id: root
    width: 800
    height: 480
    visible: true
    color: Constants.primaryBackgroundColor
    title: "JourneyOS"

    // ---------------------------------------------------------
    // 1. APP NAVIGATION STACK
    // ---------------------------------------------------------
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "MainView.qml"
    }

    // ---------------------------------------------------------
    // 2. GLOBAL POPUPS (OS LEVEL)
    // ---------------------------------------------------------
    // These now live here, so they can overlay Android Auto

    Popup {
        id: volumePopup
        x: root.width - width
        y: 100
        width: 75
        height: 380
        modal: false // Don't block input to map while changing volume
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor
            radius: 10
        }
        contentItem: VolumePopup {}

        // Auto-close logic
        Timer {
            id: volTimer; interval: 3000; running: volumePopup.opened; onTriggered: volumePopup.close()
        }
    }

    Popup {
        id: brightnessPopup
        x: root.width - width - 85 // Shift left of volume
        y: 100
        width: 75
        height: 380
        modal: false
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor
            radius: 10
        }
        contentItem: BrightnessPopup {}
    }

    Popup {
        id: bluetoothPopup
        x: 100
        y: 100
        width: 300
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: "#FFFFFF"
            border.color: "transparent"
            border.width: 0
        }
        contentItem: BluetoothPopup {
            onClose: bluetoothPopup.close()
        }
    }

    Popup {
        id: pinPopup
        width: 300
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // We will create this QML file next
        contentItem: BluetoothPinPopup {
            // We pass the agent from the C++ view model
            agent: bluetoothViewModel.agent

            onAccepted: pinPopup.close()
            onRejected: pinPopup.close()
        }
    }

    Popup {
        id: wifiPopup
        x: 100
        y: 100
        width: 300
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: "#FFFFFF"
            border.color: "transparent"
            border.width: 0
        }
        contentItem: WirelessPopup {

        }
    }

    // ---------------------------------------------------------
    // 3. LOGIC & CONNECTIONS
    // ---------------------------------------------------------

    // Handle signals from the Current View (MainView OR AndroidAutoView)
    Connections {
        target: stackView.currentItem
        ignoreUnknownSignals: true
        function onViewVolume() { volumePopup.open() }
        function onViewBrightness() { brightnessPopup.open() }
        function onViewSettings() { stackView.push("SettingsView.qml") }
        // Handle "Home" requests from any sub-screen
        function onRequestHome() { stackView.pop(null) }
    }

    // Handle Android Auto Connection
    Connections {
        target: androidAutoMonitor
        function onConnectivityStateChanged(state) {
            console.log("AA State: " + state)

            if (state === AndroidAutoConnectivityState.AA_CONNECTED) {
                console.log("Connected")
                if (stackView.currentItem.objectName !== "AndroidAutoView") {
                    stackView.push("AndroidAutoView.qml")
                }
            }
            else if (state === AndroidAutoConnectivityState.AA_DISCONNECTED) {
                // Find AA view and remove it, or just go home
                console.log("Disconnected")
                stackView.pop(null)
            }
        }
    }
}