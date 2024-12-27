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
    id: mediaPlayingMenu
    width: Constants.width
    height: Constants.height

    signal viewSettings


    Column {
        id: leftColumn
        width: 110
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        padding: 10
        clip: true
        spacing: 10

        JourneyButton {
            id: settingsButton
            text: "Settings"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/gears.svg"

            Connections {
                target: settingsButton
                onClicked: mediaPlayingMenu.viewSettings()
            }
        }

        JourneyButton {
            id: radioButton
            icon.source: "images/radio.svg"
            text: "Radio"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            Connections {
                target: radioButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: navigationButton
            text: "Navigation"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/map-marker.svg"
            Connections {
                target: navigationButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: usbButton
            text: "USB"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/usb-pendrive.svg"
            Connections {
                target: usbButton
                onClicked: console.log("clicked")
            }
        }
    }

    Column {
        id: rightColumn
        x: 600
        width: 110
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        padding: 10
        spacing: 10

        JourneyButton {
            id: obdButton
            text: "OBD"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/chart-tree.svg"
            Connections {
                target: obdButton
                onClicked: console.log("clicked")
            }
        }

        JourneyButton {
            id: powerButton
            text: "Power"
            iconSize: 36
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/power.svg"
            Connections {
                target: powerButton
                onClicked: console.log("clicked")
            }
        }

    }

}
