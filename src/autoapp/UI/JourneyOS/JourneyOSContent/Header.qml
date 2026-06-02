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
import AndroidAutoMonitor

Item {
    id: headerItem
    width: parent ? parent.width : 800
    height: parent ? parent.height : 100
    clip: true

    signal viewBluetoothStatus
    signal viewWifiStatus
    signal viewAndroidAutoStatus

    signal viewVolume
    signal viewBrightness

    // --- Wifi Status Bindings ---
    property bool hotspotEnabled: wifiViewModel.isHotspot
    property bool hasWifi: wifiViewModel.availableInterfaces.length > 0
    property bool wifiEnabled: wifiViewModel.isEnabled
    property int wifiSignalStrength: wifiViewModel.signalStrength
    property bool wifiConnected: wifiViewModel.connected
    property string wifiSsid: wifiViewModel.currentSsid
    // Compact: show SSID when connected, abbreviations otherwise
    property string wifiStatusText:
        (wifiConnected ? wifiSsid :
        (wifiEnabled ? (hotspotEnabled ? "AP" : (hasWifi ? "N/C" : "")) : "Off"))

    // Bluetooth
    property bool hasBluetooth: true
    property bool bluetoothPaired: bluetoothHandler.bluetoothConnectionStatus !== BluetoothConnectionStatus.BC_NOT_CONFIGURED
    property bool bluetoothConnected: bluetoothHandler.bluetoothConnectionStatus === BluetoothConnectionStatus.BC_CONNECTED
    property bool bluetoothConnecting: bluetoothHandler.bluetoothConnectionStatus === BluetoothConnectionStatus.BC_CONNECTING
    property string btConnectedName: bluetoothHandler.connectedDeviceName
    // Show connected device name when paired, "···" while connecting, empty otherwise
    property string bluetoothStatusText:
        (bluetoothConnected ? (btConnectedName.length > 0 ? btConnectedName : "On") :
        (bluetoothConnecting ? "···" : ""))

    // Android Auto
    property bool androidAutoConnected: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTED
    property bool androidAutoConnecting: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTING
    // Show method only when active; empty when idle
    property string androidAutoStatusText:
        (androidAutoConnecting ? "···" :
        (androidAutoConnected ? (androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_USB ? "USB"
                               : androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_WIFI ? "WiFi"
                               : "") : ""))

    Rectangle {
        color: Constants.primaryBackgroundColor
        anchors.fill: parent
    }

    Item {
        anchors.fill: parent
        anchors.margins: 10

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
                        height: Math.min(30, parent.height)
                        textIsStatus: true
                        text: headerItem.bluetoothStatusText
                        icon.source: "images/bluetooth.svg"
                        iconColor: !headerItem.bluetoothPaired ? Constants.waitColor : (headerItem.bluetoothConnecting ? Constants.actionColor : (!headerItem.bluetoothConnected ? Constants.badColor : Constants.okColor))
                        iconSize: height * 0.5
                        visible: headerItem.hasBluetooth
                        Connections {
                            target: bluetoothButton
                            onClicked: headerItem.viewBluetoothStatus()
                        }
                    }
                    JourneyButton {
                        id: wirelessButton
                        height: Math.min(30, parent.height)
                        textIsStatus: true
                        text: headerItem.wifiStatusText
                        icon.source: "images/wifi.svg"
                        iconColor: !headerItem.wifiEnabled ? Constants.badColor : (!headerItem.wifiConnected ? Constants.waitColor : Constants.okColor)
                        iconSize: height * 0.5
                        visible: headerItem.hasWifi
                        Connections {
                            target: wirelessButton
                            onClicked: headerItem.viewWifiStatus()
                        }
                    }
                    JourneyButton {
                        id: androidAutoButton
                        height: Math.min(30, parent.height)
                        textIsStatus: true
                        text: headerItem.androidAutoStatusText
                        icon.source: "images/android-auto.svg"
                        iconColor: headerItem.androidAutoConnecting ? Constants.actionColor : (headerItem.androidAutoConnected ? Constants.okColor : Constants.baseColor)
                        iconSize: height * 0.5

                        Connections {
                            target: androidAutoButton
                            onClicked: headerItem.viewAndroidAutoStatus()
                        }
                    }
                }
            }

            Image {
                id: logoImage
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width * 0.45
                height: parent.height
                source: "images/journey.png"
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
                        height: Math.min(30, parent.height)
                        width: height
                        text: ""
                        icon.source: "images/brightness.svg"
                        iconSize: height * 0.5
                        Connections {
                            target: brightnessButton
                            onClicked: headerItem.viewBrightness()
                        }
                    }

                    JourneyButton {
                        id: volumeButton
                        height: Math.min(30, parent.height)
                        width: height
                        text: ""
                        icon.source: "images/volume.svg"
                        iconSize: height * 0.5
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
