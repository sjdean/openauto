import QtQuick
import QtQuick.Controls
import QtQuick.VirtualKeyboard
import JourneyOS 1.0
import AndroidAutoMonitor

Window {
    id: root
    width: 800
    height: 480
    visible: true
    visibility: settingsViewHandler.headUnitMode ? Window.FullScreen : Window.Windowed
    color: Constants.primaryBackgroundColor
    title: "JourneyOS"

    // Bind the global dark/light mode toggle to the persisted setting
    Binding { target: Constants; property: "isDarkMode";            value: settingsViewHandler.uiDarkMode }
    Binding { target: Constants; property: "accentPrimary";         value: settingsViewHandler.uiAccentPrimary }
    Binding { target: Constants; property: "accentBrand2";          value: settingsViewHandler.uiAccentBrand2 }
    Binding { target: Constants; property: "buttonSurfaceOpacity";  value: settingsViewHandler.uiButtonOpacity }

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
        y: root.height * 0.2
        width: Math.min(75, root.width * 0.1)
        height: root.height * 0.7
        modal: false // Don't block input to map while changing volume
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor
            radius: Constants.radiusPopup
        }
        contentItem: VolumePopup {}
        onClosed: {
            volumePopupHandler.saveSettings()
        }
        onOpened: volTimer.restart()
        // Auto-close: 5 s after last interaction (restarted by Connections below)
        Timer {
            id: volTimer; interval: 5000; repeat: false; onTriggered: volumePopup.close()
        }
    }

    Popup {
        id: brightnessPopup
        x: root.width - (Math.min(75, root.width * 0.1) * 2) - (root.width * 0.02) // Left of volume, no cross-popup dependency
        y: root.height * 0.2
        width: Math.min(75, root.width * 0.1)
        height: root.height * 0.7
        modal: false
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor
            radius: Constants.radiusPopup
        }
        contentItem: BrightnessPopup {}
        onClosed: {
            brightnessPopupHandler.saveSettings()
        }
        onOpened: brtTimer.restart()
        // Auto-close: 5 s after last interaction (restarted by Connections below)
        Timer {
            id: brtTimer; interval: 5000; repeat: false; onTriggered: brightnessPopup.close()
        }
    }

    Popup {
        id: bluetoothPopup
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        modal: true
        focus: true
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Item {}
        contentItem: BluetoothPopup {
            onClose: bluetoothPopup.close()
        }
    }

    Popup {
        id: wifiPopup
        anchors.centerIn: parent
        width: Math.min(500, parent.width - 40)
        height: Math.min(wifiPopupContent.implicitHeight + 20, parent.height - 40)
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0
        background: Item {}
        contentItem: WirelessPopup {
            id: wifiPopupContent
            width: wifiPopup.width
            onClose: wifiPopup.close()
        }
    }

    AndroidAutoStatusPopup {
        id: aaInfoPopup
        androidAutoConnected: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTED
        androidAutoConnecting: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTING
        androidAutoMethodText: androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_USB ? "USB" : "Wi-Fi"
        onViewAndroidAutoRequested: {
            if (stackView.currentItem.objectName !== "AndroidAutoView")
                stackView.push("AndroidAutoView.qml")
        }
    }

    PowerPopup {
        id: powerPopup
        onRebootRequested: systemPower.reboot()
        onShutdownRequested: systemPower.powerOff()
    }

    // Steering-wheel button toast — auto-dismisses after 2 s.
    // canBusReceiver may be null on desktop (non-CAN) builds.
    Rectangle {
        id: buttonToast
        visible: false
        z: 9998
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.08
        width: toastLabel.implicitWidth + 32
        height: toastLabel.implicitHeight + 16
        radius: height / 2
        color: Qt.rgba(0, 0, 0, 0.72)

        Text {
            id: toastLabel
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 14
            text: (canBusReceiver && canBusReceiver.lastButtonPressed) || ""
        }

        Timer {
            id: toastTimer
            interval: 2000
            repeat: false
            onTriggered: buttonToast.visible = false
        }
    }

    Connections {
        target: canBusReceiver
        ignoreUnknownSignals: true
        function onLastButtonPressedChanged() {
            if (canBusReceiver && canBusReceiver.lastButtonPressed !== "") {
                buttonToast.visible = true
                toastTimer.restart()
            }
        }
    }

    // ---------------------------------------------------------
    // 3. LOGIC & CONNECTIONS
    // ---------------------------------------------------------

    // Handle signals from the current view (MainView OR AndroidAutoView)
    Connections {
        target: stackView.currentItem
        ignoreUnknownSignals: true

        // Navigation
        function onViewPower()       { powerPopup.open() }
        function onViewOBD()         { stackView.push("CanBusDashboard.qml") }
        function onViewNavigation()  { console.log("Navigation Clicked") } // TODO: Nav screen
        function onViewRadio()       { console.log("Radio Clicked") }     // TODO: Radio screen
        function onViewUSB()         { console.log("USB Clicked") }       // TODO: USB browser
        function onViewBluetooth()   { console.log("Bluetooth Clicked") } // TODO: BT HFP/A2DP
        function onViewSettings()    { stackView.push("SettingsView.qml") }
        function onViewAndroidAuto() { stackView.push("AndroidAutoView.qml") }
        function onRequestHome()     { stackView.pop(null) }

        // Status popups
        function onViewWifiStatus()          { wifiPopup.open() }
        function onViewBluetoothStatus()     { bluetoothPopup.open() }
        function onViewAndroidAutoStatus()   { aaInfoPopup.open() }

        // Slider popups
        function onViewVolume()     { volumePopup.open() }
        function onViewBrightness() { brightnessPopup.open() }
    }

    // Volume popup: open (or stay open) whenever volume or mute changes —
    // covers both touch-slider interactions and CAN steering-wheel buttons.
    Connections {
        target: volumePopupHandler
        function onVolumeSinkChanged()     { volumePopup.open(); volTimer.restart() }
        function onVolumeSinkMuteChanged() { volumePopup.open(); volTimer.restart() }
    }
    Connections {
        target: brightnessPopupHandler
        function onTargetBrightnessChanged() { if (brightnessPopup.opened) brtTimer.restart() }
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

    // ---------------------------------------------------------
    // VIRTUAL KEYBOARD
    // ---------------------------------------------------------
    // InputPanel must live in the root Window so it can overlay all content.
    // It starts off-screen (y: root.height) and slides up when a text field
    // is focused, then slides back down on dismiss.
    InputPanel {
        id: inputPanel
        z: 10000   // above softwareDimmer (9999) so it remains readable at low brightness
        x: 0
        width: root.width
        y: root.height  // off-screen when idle

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges { target: inputPanel; y: root.height - inputPanel.height }
        }
        transitions: Transition {
            from: ""; to: "visible"; reversible: true
            NumberAnimation { property: "y"; duration: 250; easing.type: Easing.InOutQuad }
        }
    }

    Rectangle {
        id: softwareDimmer
        anchors.fill: parent
        color: "black"
        z: 9999 // Ensure it is above Popups and Android Auto

        // Transparent to mouse events so you can still touch buttons!
        enabled: false

        // Calculate Opacity:
        // Brightness 255 (Max) -> Opacity 0.0 (Invisible)
        // Brightness 0   (Min) -> Opacity 0.85 (Very Dark, but legible)
        // We don't go to 1.0, otherwise you can't find the slider to fix it!
        // Divide by currentMax so that at max slider position opacity == 0 (no tint)
        opacity: brightnessPopupHandler.currentMax > 0
                 ? 0.85 * Math.max(0.0, 1.0 - (brightnessPopupHandler.screenBrightness / brightnessPopupHandler.currentMax))
                 : 0

        // Smooth transition when changing brightness
        Behavior on opacity { NumberAnimation { duration: 100 } }
    }
}
