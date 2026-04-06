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
    width: parent ? parent.width : Constants.width
    height: parent ? parent.height : Constants.height
    property bool showSettings: true;
    property bool hasRadio: false;
    property bool hasNavigation: false;
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

    Column {
        id: leftColumn
        width: parent.width * 0.15
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        padding: 10
        spacing: 10
        clip: true

        JourneyButton {
            id: settingsButton
            text: "Settings"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/settings.svg"
            visible: mediaPlayingMenu.showSettings

            Connections {
                target: settingsButton
                onClicked: mediaPlayingMenu.viewSettings()
            }
        }

        JourneyButton {
            id: androidAutoButton
            text: "Android Auto"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/android-auto.svg"
            visible: mediaPlayingMenu.hasAndroidAuto

            Connections {
                target: androidAutoButton
                onClicked: mediaPlayingMenu.viewAndroidAuto()
            }
        }

        JourneyButton {
            id: radioButton
            icon.source: "images/radio.svg"
            text: "Radio"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            visible: mediaPlayingMenu.hasRadio
            Connections {
                target: radioButton
                onClicked: mediaPlayingMenu.viewRadio()
            }
        }

        JourneyButton {
            id: navigationButton
            text: "Navigation"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/map-marker.svg"
            visible: mediaPlayingMenu.hasNavigation
            Connections {
                target: navigationButton
                onClicked: mediaPlayingMenu.viewNavigation()
            }
        }

        JourneyButton {
            id: usbButton
            text: "USB"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/usb.svg"
            visible: mediaPlayingMenu.hasUSB
            Connections {
                target: usbButton
                onClicked: mediaPlayingMenu.viewUSB()
            }
        }
    }

    Column {
        id: rightColumn
        width: parent.width * 0.15
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
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/chart-tree.svg"
            visible: mediaPlayingMenu.hasOBD
            Connections {
                target: obdButton
                onClicked: mediaPlayingMenu.viewOBD()
            }
        }

        JourneyButton {
            id: powerButton
            text: "Power"
            iconSize: parent.width * 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: "images/power.svg"
            visible: mediaPlayingMenu.showPower
            Connections {
                target: powerButton
                onClicked: mediaPlayingMenu.viewPower()
            }
        }
    }
}
