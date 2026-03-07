import QtQuick
import QtQuick.Controls
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
        onClosed: {
            volumePopupHandler.saveSettings()
        }
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
        onClosed: {
            brightnessPopupHandler.saveSettings()
        }
        // Auto-close logic
        Timer {
            id: brtTimer; interval: 3000; running: brightnessPopup.opened; onTriggered: brightnessPopup.close()
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
        }
    }

    Popup {
        id: powerPopup
        anchors.centerIn: parent
        width: 400
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: Constants.settingsPopupBackgroundColor
            radius: 10
            border.color: Constants.primaryBackgroundColor
            border.width: 1
        }

        Column {
            anchors.centerIn: parent
            spacing: 20

            Text {
                text: "System Power"
                font.pixelSize: 24
                color: Constants.primaryTextColor
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 20
                JourneyButton {
                    text: "Reboot"
                    width: 150
                    height: 50
                    onClicked: systemPower.reboot()
                }
                JourneyButton {
                    text: "Shutdown"
                    width: 150
                    height: 50
                    onClicked: systemPower.powerOff()
                }
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
        function onViewOBD()         { console.log("OBD Clicked") }       // TODO: OBD screen
        function onViewNavigation()  { console.log("Navigation Clicked") } // TODO: Nav screen
        function onViewRadio()       { console.log("Radio Clicked") }     // TODO: Radio screen
        function onViewUSB()         { console.log("USB Clicked") }       // TODO: USB browser
        function onViewBluetooth()   { console.log("Bluetooth Clicked") } // TODO: BT HFP/A2DP
        function onViewSettings()    { stackView.push("SettingsView.qml") }
        function onViewAndroidAuto() { stackView.push("AndroidAutoView.qml") }
        function onRequestHome()     { stackView.pop(null) }

        // Status popups
        function onViewWifiStatus()      { wifiPopup.open() }
        function onViewBluetoothStatus() { bluetoothPopup.open() }

        // Slider popups
        function onViewVolume()     { volumePopup.open() }
        function onViewBrightness() { brightnessPopup.open() }
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
        opacity: 0.85 * (1.0 - (brightnessPopupHandler.screenBrightness / 255.0))

        // Smooth transition when changing brightness
        Behavior on opacity { NumberAnimation { duration: 100 } }
    }
}