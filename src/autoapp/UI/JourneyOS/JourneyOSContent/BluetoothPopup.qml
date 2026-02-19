import QtQuick
import QtQuick.Controls
import JourneyOS
import AndroidAutoMonitor

Item {
    id: root
    width: 800
    height: 480

    signal close

    // ── PIN / Passkey confirmation overlay (Linux only, safe no-op on macOS) ──
    Rectangle {
        id: pinOverlay
        anchors.fill: parent
        color: "#CC000000"
        z: 20
        visible: false

        property string promptText: ""

        Column {
            anchors.centerIn: parent
            spacing: 20

            Rectangle {
                width: 420
                height: 180
                radius: 10
                color: Constants.primaryBackgroundColor
                border.color: Constants.waitColor
                border.width: 2
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: pinOverlay.promptText
                        color: "white"
                        font.pixelSize: 18
                        wrapMode: Text.WordWrap
                        width: 380
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20

                        JourneyButton {
                            text: "Accept"
                            onClicked: {
                                if (bluetoothHandler.agent)
                                    bluetoothHandler.agent.accept()
                                pinOverlay.visible = false
                            }
                        }

                        JourneyButton {
                            text: "Decline"
                            onClicked: {
                                if (bluetoothHandler.agent)
                                    bluetoothHandler.agent.reject()
                                pinOverlay.visible = false
                            }
                        }
                    }
                }
            }
        }
    }

    // Wire agent signals — Connections with a null target is safe in Qt6
    Connections {
        target: bluetoothHandler.agent
        ignoreUnknownSignals: true

        function onShowConfirmation(passkey) {
            pinOverlay.promptText = "Confirm passkey on both devices:\n" + passkey
            pinOverlay.visible = true
        }

        function onShowPinCode(pin) {
            pinOverlay.promptText = "Enter this PIN on the device:\n" + pin
            pinOverlay.visible = true
        }

        function onPairingComplete() {
            pinOverlay.visible = false
        }
    }

    // ── Background ────────────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: Constants.primaryBackgroundColor

        // ── System-managed status view (Mac/Windows/Linux Desktop) ───────────
        Column {
            anchors.centerIn: parent
            spacing: 16
            visible: !settingsViewHandler.headUnitMode

            Rectangle {
                width: 16
                height: 16
                radius: 8
                anchors.horizontalCenter: parent.horizontalCenter
                color: {
                    var s = bluetoothHandler.bluetoothConnectionStatus
                    if (s === BluetoothConnectionStatus.BC_CONNECTED)    return Constants.okColor
                    if (s === BluetoothConnectionStatus.BC_CONNECTING)   return Constants.waitColor
                    if (s === BluetoothConnectionStatus.BC_DISCONNECTED) return Constants.actionColor
                    return Constants.badColor
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Bluetooth — " + bluetoothHandler.statusText
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Bluetooth is managed by the operating system.\nEnable Head Unit Mode in Settings › System to manage it here."
                color: "lightgray"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: 500
            }
        }

        // ── Full management view (Head Unit mode) ─────────────────────────────
        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12
            visible: settingsViewHandler.headUnitMode

            // ── Status row ────────────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Bluetooth"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Coloured status dot
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    anchors.verticalCenter: parent.verticalCenter
                    color: {
                        var s = bluetoothHandler.bluetoothConnectionStatus
                        if (s === BluetoothConnectionStatus.BC_CONNECTED)   return Constants.okColor
                        if (s === BluetoothConnectionStatus.BC_CONNECTING)  return Constants.waitColor
                        if (s === BluetoothConnectionStatus.BC_DISCONNECTED) return Constants.actionColor
                        return Constants.badColor
                    }
                }

                Text {
                    text: bluetoothHandler.statusText
                    color: "lightgray"
                    font.pixelSize: 14
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Adapter selection (only shown when multiple adapters exist) ───
            Row {
                width: parent.width
                spacing: 10
                visible: bluetoothHandler.adapterCount > 1

                Text {
                    text: "Adapter:"
                    color: "lightgray"
                    font.pixelSize: 13
                    anchors.verticalCenter: parent.verticalCenter
                }

                ComboBox {
                    id: adapterCombo
                    width: parent.width - 80
                    property var adapterList: bluetoothHandler.bluetoothAdapterList
                    model: adapterList
                    textRole: "name"
                    onCurrentIndexChanged: {
                        if (currentIndex >= 0 && currentIndex < adapterList.length)
                            bluetoothHandler.setActiveAdapter(adapterList[currentIndex].address)
                    }
                }
            }

            // ── Paired devices ────────────────────────────────────────────────
            Text {
                text: "Paired Devices"
                color: "lightgray"
                font.pixelSize: 13
                font.bold: true
            }

            ListView {
                id: pairedList
                width: parent.width
                height: 150
                clip: true
                model: bluetoothHandler.pairedDeviceList

                delegate: Rectangle {
                    width: pairedList.width
                    height: 48
                    color: "transparent"

                    // Device name + connected indicator
                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Text {
                            text: model.name || model.address
                            color: "white"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            visible: model.connected
                            text: "(Connected)"
                            color: Constants.okColor
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Action buttons
                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        JourneyButton {
                            text: model.connected ? "Disconnect" : "Connect"
                            onClicked: {
                                if (model.connected)
                                    bluetoothHandler.doDisconnect(model.address)
                                else
                                    bluetoothHandler.doConnectToPairedDevice(model.address)
                            }
                        }

                        JourneyButton {
                            text: "Forget"
                            onClicked: bluetoothHandler.doRemovePair(model.address)
                        }
                    }
                }

                // Empty state
                Text {
                    visible: pairedList.count === 0
                    anchors.centerIn: parent
                    text: "No paired devices"
                    color: "gray"
                    font.pixelSize: 13
                }
            }

            // ── Scan controls ─────────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 12

                JourneyButton {
                    text: bluetoothHandler.isScanning ? "Scanning…" : "Scan for Devices"
                    enabled: !bluetoothHandler.isScanning
                    onClicked: bluetoothHandler.startScan()
                }

                Text {
                    visible: bluetoothHandler.isScanning
                    text: "Searching for devices…"
                    color: Constants.waitColor
                    font.pixelSize: 13
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Available / scanned devices ───────────────────────────────────
            Text {
                visible: availableList.count > 0 || bluetoothHandler.isScanning
                text: "Available Devices"
                color: "lightgray"
                font.pixelSize: 13
                font.bold: true
            }

            ListView {
                id: availableList
                width: parent.width
                height: 120
                clip: true
                visible: count > 0
                model: bluetoothHandler.unpairedDeviceList

                delegate: Rectangle {
                    width: availableList.width
                    height: 44
                    color: "transparent"

                    Text {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.name || model.address
                        color: "white"
                        font.pixelSize: 14
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        JourneyButton {
                            text: "Pair"
                            onClicked: bluetoothHandler.pair(model.address)
                        }

                        JourneyButton {
                            text: "Ignore"
                            onClicked: bluetoothHandler.ignoreDevice(model.address)
                        }
                    }
                }
            }
        }
    }
}