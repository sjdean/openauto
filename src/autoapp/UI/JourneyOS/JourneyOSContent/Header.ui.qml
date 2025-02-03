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
import AndroidAutoMonitor 1.0

Item {
    id: headerItem
    width: 800
    height: 278
    clip: true

    signal viewBluetoothStatus
    signal viewWifiStatus
    signal viewAndroidAutoStatus

    signal viewVolume
    signal viewBrightness

    property bool hasBluetooth: true
    property bool hasWifi: true

    property string bluetoothStatusText: bluetoothPopupHandler.statusText
    property bool bluetoothPaired: bluetoothPopupHandler.bluetoothStatus = !BluetoothConnectionStatus.BC_NOT_CONFIGURED
    property bool bluetoothConnected: bluetoothPopupHandler.bluetoothStatus = BluetoothConnectionStatus.BC_CONNECTED
    property bool bluetoothConnecting: bluetoothPopupHandler.bluetoothStatus = BluetoothConnectionStatus.BC_CONNECTING
    //TODO: Wifi Connectivity Settings
    property bool wifiPaired: false
    property bool wifiConnected: false
    property bool wifiConnecting: false

    property bool androidAutoConnected: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTED
    property bool androidAutoConnecting: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTING
    property string androidAutoMethodText: androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_USB ? "USB"
                                                                  : AndroidAutoConnectivityMethod.AA_WIFI ? "Wi-Fi"
                                                                  : "Unknown"

    Rectangle {
        color: Constants.primaryBackgroundColor
        anchors.fill: parent
    }

    Item {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10

        Row {
            id: row
            width: parent.width
            height: parent.height

            Item {
                id: itemLeft
                width: (parent.width - logoImage.width) / 2
                height: parent.height
                Row {
                    id: rowLeft
                    height: parent.height
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    spacing: 5

                    JourneyButton {
                        id: bluetoothButton
                        height: 25
                        textIsStatus: true
                        text: headerItem.bluetoothStatusText
                        icon.source: "images/bluetooth-alt.svg"
                        iconColor: !headerItem.bluetoothPaired ? Constants.waitColor : (headerItem.bluetoothConnecting ? Constants.actionColor : (!headerItem.bluetoothConnected ? Constants.badColor : Constants.okColor))
                        iconSize: 12
                        visible: headerItem.hasBluetooth
                        Connections {
                            target: bluetoothButton
                            onClicked: headerItem.viewBluetoothStatus()
                        }
                    }
                    JourneyButton {
                        id: wirelessButton
                        height: 25
                        textIsStatus: true
                        text: "Status"
                        icon.source: "images/fi-br-wifi.svg"
                        iconColor: !headerItem.wifiPaired ? Constants.waitColor : (headerItem.wifiConnecting ? Constants.actionColor : (!headerItem.wifiConnected ? Constants.badColor : Constants.okColor))
                        iconSize: 12
                        visible: headerItem.hasWifi
                        Connections {
                            target: wirelessButton
                            onClicked: headerItem.viewWifiStatus()
                        }
                    }
                    JourneyButton {
                        id: androidAutoButton
                        height: 25
                        textIsStatus: true
                        text: headerItem.androidAutoMethodText
                        icon.source: "images/android-auto.svg"
                        iconColor: headerItem.androidAutoConnecting ? Constants.actionColor : (headerItem.androidAutoConnected ? Constants.okColor : Constants.baseColor)
                        iconSize: 12

                        Connections {
                            target: androidAutoButton
                            onClicked: headerItem.viewAndroidAutoStatus()
                        }
                    }
                }
            }



            Image {
                id: logoImage
                y: -86
                width: 366
                height: 278
                source: "images/journey.jpg"
                fillMode: Image.PreserveAspectFit
            }
            Item {
                id: itemRight
                width: (parent.width - logoImage.width) / 2
                height: parent.height
                Row {
                    id: rowRight
                    height: parent.height
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    spacing: 5

                    JourneyButton {
                        id: brightnessButton
                        width: 25
                        height: 25
                        text: ""
                        icon.source: "images/fi-br-brightness.svg"
                        iconSize: 12
                        Connections {
                            target: brightnessButton
                            onClicked: headerItem.viewBrightness()
                        }
                    }

                    JourneyButton {
                        id: volumeButton
                        width: 25
                        height: 25
                        text: ""
                        icon.source: "images/volume.svg"
                        iconSize: 12
                        Connections {
                            target: volumeButton
                            onClicked: headerItem.viewVolume()
                        }

                    }
                }
            }
        }

    }

}
