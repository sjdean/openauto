import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS
import AndroidAutoMonitor

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

    // --- Wifi Status Bindings ---
    property bool hotspotEnabled: wifiViewModel.isHotspot
    property bool hasWifi: wifiViewModel.availableInterfaces.count > 0
    property bool wifiEnabled: wifiViewModel.isEnabled
    property int wifiSignalStrength: wifiViewModel.signalStrength
    property bool wifiConnected: wifiViewModel.connected
    property string wifiSsid: wifiViewModel.currentSsid
    property string wifiStatusText:
        (wifiConnected ? wifiSsid :
        (wifiEnabled ? (hotspotEnabled ? "Listening for Connections" : (hasWifi ? "Not Connected" : "No Interfaces")
        ) : "Disabled"))

        //"Wi-Fi"     // Determine what to show - probably Mac/IP of Adapter on Boot, Update Icon to a Configured colour,
                                                // Then monitor for connection, show "Connected to (SSID)"
                                                // Then perhaps roll back to SSID and/or Client ID connected to

    // Bluetooth - This is to display minimal Bluetooth Status information for a context button
    property bool hasBluetooth: true
    property string bluetoothStatusText: bluetoothHandler.statusText
    property bool bluetoothPaired: bluetoothHandler.bluetoothConnectionStatus !== BluetoothConnectionStatus.BC_NOT_CONFIGURED
    property bool bluetoothConnected: bluetoothHandler.bluetoothConnectionStatus === BluetoothConnectionStatus.BC_CONNECTED
    property bool bluetoothConnecting: bluetoothHandler.bluetoothConnectionStatus === BluetoothConnectionStatus.BC_CONNECTING


    // Android Auto - This is to display minimal AndroidAuto status information, connectivity method, status, and eventually Device Name connected. Though we could roll that back after five seconds, or have a scrolling text
    property bool androidAutoConnected: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTED
    property bool androidAutoConnecting: androidAutoMonitor.state === AndroidAutoConnectivityState.AA_CONNECTING
    property string androidAutoMethodText: androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_USB ? "USB"
                                                                  : androidAutoMonitor.method === AndroidAutoConnectivityMethod.AA_WIFI ? "Wi-Fi"
                                                                  : "Unknown"

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
                        height: 25
                        textIsStatus: true
                        text: headerItem.bluetoothStatusText
                        icon.source: "images/bluetooth.svg"
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
                        text: headerItem.wifiStatusText
                        icon.source: "images/wifi.svg"
                        iconColor: !headerItem.wifiEnabled ? Constants.badColor : (!headerItem.wifiConnected ? Constants.waitColor : Constants.okColor)
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
                        icon.source: "images/brightness.svg"
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
