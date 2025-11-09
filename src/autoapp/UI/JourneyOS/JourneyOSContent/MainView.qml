// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: mainView
    width: 800
    height: 480

    visible: true

    property bool isMediaPlaying: false
    signal viewSettings

    Header {
        id: header
        height: 100
        Connections {
            target: header
            onViewVolume: volumePopup.open()
        }

        Connections {
            target: header
            onViewBrightness: brightnessPopup.open()
        }

        Connections {
            target: header
            onViewBluetoothStatus: bluetoothPopup.open()
        }

        Connections {
            target: header
            onViewWifiStatus: wifiPopup.open()
        }
    }

    Popup {
        id: bluetoothPopup
        x: 100
        y: 100
        width: 300
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
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
            closePolicy: Popup.CloseOnEscape

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
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            color: "#FFFFFF"
            border.color: "transparent"
            border.width: 0
        }
        contentItem: WirelessPopup {

        }
    }

    Popup {
        id: volumePopup
        x: mainView.width - width
        y: 100
        width: 75
        height: 380
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor // or whatever background color you prefer
            border.color: "transparent"
            border.width: 0
        }
        contentItem: VolumePopup {

        }
    }

    Popup {
        id: brightnessPopup
        x: mainView.width - width - volumePopup.width
        y: 100
        width: 75
        height: 380
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            color: Constants.sliderPopupBackgroundColor // or whatever background color you prefer
            border.color: "transparent"
            border.width: 0
        }
        contentItem: BrightnessPopup {

        }
    }


    // Container for menus to handle transitions
    Item {
        id: menuContainer
        width: 800
        height: 380
        anchors.top: parent.top
        anchors.topMargin: 100

        // Use states to control which menu is visible
        states: [
            State {
                name: "mainMenu"
                when: !mainView.isMediaPlaying
                PropertyChanges { target: mainMenu; opacity: 1 }
                PropertyChanges { target: mediaMenu; opacity: 0 }
            },
            State {
                name: "mediaMenu"
                when: mainView.isMediaPlaying
                PropertyChanges { target: mainMenu; opacity: 0 }
                PropertyChanges { target: mediaMenu; opacity: 1 }
            }
        ]

        // Define transitions for smooth change between states
        transitions: [
            Transition {
                from: "*"; to: "*"
                ParallelAnimation {
                    NumberAnimation { properties: "opacity"; duration: 500; easing.type: Easing.InOutQuad }
                }
            }
        ]

        MediaNotPlayingMenu {
            id: mainMenu
            width: 660
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 0 : 1 // Initial opacity based on isMediaPlaying
            Connections {
                target: mainMenu
                onViewSettings: mainView.viewSettings()

            }
        }

        MediaPlayingMenu {
            id: mediaMenu
            width: 800
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 1 : 0 // Initial opacity based on isMediaPlaying
            Connections {
                target: mediaMenu
                onViewSettings: mainView.viewSettings()

            }
        }
    }



    Connections {
            // We listen to the agent for pairing requests
            target: bluetoothViewModel.agent

            // When the agent emits, we open the PIN popup
            function onShowConfirmation(passkey) {
                pinPopup.contentItem.pinText = "Confirm passkey: " + passkey
                pinPopup.open()
            }

            function onShowPinCode(pincode) {
                pinPopup.contentItem.pinText = "Enter this PIN on your device: " + pincode
                pinPopup.open()
            }

            function onPairingComplete() {
                pinPopup.close()
            }
        }

}


