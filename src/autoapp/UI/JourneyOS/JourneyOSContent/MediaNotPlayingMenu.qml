/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS

Item {
    id: mediaNotPlayingMenu
    width: parent ? parent.width : Constants.width
    height: parent ? parent.height : Constants.height

    property bool showSettings: true;
    property bool hasRadio: false;
    property bool hasNavigation: false;
    property bool hasBluetooth: false;
    property bool hasUSB: false;
    property bool hasOBD: false;
    property bool showPower: true;
    property bool hasAndroidAuto: false;

    signal viewSettings()
    signal viewAndroidAuto()
    signal viewPower()
    signal viewOBD()
    signal viewNavigation()
    signal viewUSB()
    signal viewRadio()
    signal viewBluetooth()

    Rectangle {
        anchors.fill: parent
        color: "#00000000"
    }

    Rectangle {
        width: parent.width - 200
        height: parent.height - 200
        color: "#00000000"

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Grid {
            id: menuGrid
            width: parent.width
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10
            padding: 5

            // Media Sources
            JourneyButton {
                id: androidAutoButton
                icon.source: "images/radio.svg"
                text: "Android Auto"
                visible: mediaNotPlayingMenu.hasAndroidAuto
                Connections {
                    target: androidAutoButton
                    onClicked: mediaNotPlayingMenu.viewAndroidAuto()
                }
            }

            JourneyButton {
                id: radioButton
                icon.source: "images/radio.svg"
                text: "Radio"
                visible: mediaNotPlayingMenu.hasRadio
                Connections {
                    target: radioButton
                    onClicked: mediaNotPlayingMenu.viewRadio()
                }
            }

            JourneyButton {
                id: bluetoothButton
                text: "Bluetooth"
                icon.source: "images/bluetooth-alt.svg"
                visible: mediaNotPlayingMenu.hasBluetooth
                Connections {
                    target: bluetoothButton
                    onClicked: mediaNotPlayingMenu.viewBluetooth()
                }
            }

            JourneyButton {
                id: usbButton
                text: "USB"
                icon.source: "images/usb-pendrive.svg"
                visible: mediaNotPlayingMenu.hasUSB
                Connections {
                    target: usbButton
                    onClicked: mediaNotPlayingMenu.viewUSB()
                }
            }

            // Functions - Navigation
            JourneyButton {
                id: navigationButton
                text: "Navigation"
                icon.source: "images/map-marker.svg"
                visible: mediaNotPlayingMenu.hasNavigation
                Connections {
                    target: navigationButton
                    onClicked: mediaNotPlayingMenu.viewNavigation()
                }
            }

            // Functions - OBD
            JourneyButton {
                id: obdButton
                text: "OBD"
                icon.source: "images/chart-tree.svg"
                visible: mediaNotPlayingMenu.hasOBD
                Connections {
                    target: obdButton
                    onClicked: mediaNotPlayingMenu.viewODB()
                }
            }

            // Functions - System
            JourneyButton {
                id: settingsButton
                text: "Settings"
                icon.source: "images/gears.svg"
                visible: mediaNotPlayingMenu.showSettings

                Connections {
                    target: settingsButton
                    onClicked: mediaNotPlayingMenu.viewSettings()
                }
            }

            JourneyButton {
                id: powerButton
                text: "Power"
                icon.source: "images/power.svg"
                visible: mediaNotPlayingMenu.showPower
                Connections {
                    target: powerButton
                    onClicked: mediaNotPlayingMenu.viewPower()
                }
            }
        }
    }
}
