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
    width: Constants.width
    height: Constants.height

    signal viewSettings


    Grid {
        id: menuGrid
        width: Constants.width - 200    // TODO: Dynamic Sizing of Left/Right Columns and Buttons based on Screen Size
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        padding: 5

        JourneyButton {
            id: settingsButton
            text: "Settings"
            icon.source: "images/gears.svg"

            Connections {
                target: settingsButton
                onClicked: mediaNotPlayingMenu.viewSettings()
            }
        }

        JourneyButton {
            id: radioButton
            icon.source: "images/radio.svg"
            text: "Radio"
            Connections {
                target: radioButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: navigationButton
            text: "Navigation"
            icon.source: "images/map-marker.svg"
            Connections {
                target: navigationButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: bluetoothButton
            text: "Bluetooth"
            icon.source: "images/bluetooth-alt.svg"
            Connections {
                target: bluetoothButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: usbButton
            text: "USB"
            icon.source: "images/usb-pendrive.svg"
            Connections {
                target: usbButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: powerButton
            text: "Power"
            icon.source: "images/power.svg"
            Connections {
                target: powerButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: obdButton
            text: "OBD"
            icon.source: "images/chart-tree.svg"
            Connections {
                target: obdButton
                onClicked: console.log("clicked")
            }
        }
    }

}
