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
        color: Constants.popupOverlayDim
        z: 20
        visible: false

        property string promptText: ""

        Column {
            anchors.centerIn: parent
            spacing: 20

            Rectangle {
                width: 420
                height: 180
                radius: Constants.radiusPopup
                color: Constants.popupBackground
                border.color: Constants.waitColor
                border.width: 2
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: pinOverlay.promptText
                        color: Constants.textPrimary
                        font.pixelSize: Constants.fontHeading
                        wrapMode: Text.WordWrap
                        width: 380
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20

                        JourneyButton {
                            text: "Accept"
                            icon.source: "images/check.svg"
                            onClicked: {
                                if (bluetoothHandler.agent)
                                    bluetoothHandler.agent.accept()
                                pinOverlay.visible = false
                            }
                        }

                        JourneyButton {
                            text: "Decline"
                            icon.source: "images/close.svg"
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

    // ── Remove device confirmation overlay ────────────────────────────────────
    Rectangle {
        id: removeConfirmOverlay
        anchors.fill: parent
        color: Constants.popupOverlayDim
        z: 10
        visible: false

        property string deviceAddress: ""
        property string deviceName: ""

        Column {
            anchors.centerIn: parent
            spacing: 20

            Rectangle {
                width: 380
                height: 160
                radius: Constants.radiusPopup
                color: Constants.popupBackground
                border.color: Constants.statusBad
                border.width: 2
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Forget \"" + removeConfirmOverlay.deviceName + "\"?"
                        color: Constants.textPrimary
                        font.pixelSize: Constants.fontSubtitle
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: 340
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20

                        JourneyButton {
                            text: "Forget"
                            icon.source: "images/trash.svg"
                            onClicked: {
                                bluetoothHandler.removePair(removeConfirmOverlay.deviceAddress)
                                removeConfirmOverlay.visible = false
                            }
                        }

                        JourneyButton {
                            text: "Cancel"
                            icon.source: "images/close.svg"
                            onClicked: removeConfirmOverlay.visible = false
                        }
                    }
                }
            }
        }
    }

    // ── Background ────────────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: Constants.popupBackground

        // Close button — small ✕ circle in top-right corner
        Rectangle {
            id: closeBtn
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            width: 28
            height: 28
            radius: Constants.radiusCircle
            color: closeBtnArea.pressed ? Constants.btnDangerBgPressed : Constants.btnDangerBg
            z: 1

            Behavior on color { ColorAnimation { duration: 80 } }

            Text {
                anchors.centerIn: parent
                text: "\u2715"
                color: Constants.btnDangerFg
                font.pixelSize: Constants.fontLabel
                font.bold: true
            }

            MouseArea {
                id: closeBtnArea
                anchors.fill: parent
                onClicked: root.close()
            }
        }

        // ── System-managed status view (Mac/Windows/Linux Desktop) ───────────
        Column {
            anchors.centerIn: parent
            spacing: 16
            visible: !ConfigGate.showConfig

            Rectangle {
                width: 16
                height: 16
                radius: 8
                anchors.horizontalCenter: parent.horizontalCenter
                color: {
                    var s = bluetoothHandler.bluetoothConnectionStatus
                    if (s === BluetoothConnectionStatus.BC_CONNECTED)    return Constants.statusOk
                    if (s === BluetoothConnectionStatus.BC_CONNECTING)   return Constants.statusWait
                    if (s === BluetoothConnectionStatus.BC_DISCONNECTED) return Constants.statusAction
                    return Constants.statusBad
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Bluetooth — " + bluetoothHandler.statusText
                color: Constants.textPrimary
                font.pixelSize: Constants.fontHeading
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Bluetooth is managed by the operating system.\nEnable Head Unit Mode in Settings › System to manage it here."
                color: Constants.textSecondary
                font.pixelSize: Constants.fontLabel
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: 500
            }
        }

        // ── Full management view (Head Unit mode) ─────────────────────────────
        Column {
            anchors.fill: parent
            anchors.margins: 16
            anchors.topMargin: 44   // leave room for close button
            spacing: 12
            visible: ConfigGate.showConfig

            // ── Status row ────────────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Bluetooth"
                    color: Constants.textPrimary
                    font.pixelSize: Constants.fontTitle
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
                        if (s === BluetoothConnectionStatus.BC_CONNECTED)    return Constants.statusOk
                        if (s === BluetoothConnectionStatus.BC_CONNECTING)   return Constants.statusWait
                        if (s === BluetoothConnectionStatus.BC_DISCONNECTED) return Constants.statusAction
                        return Constants.statusBad
                    }
                }

                Text {
                    text: bluetoothHandler.statusText
                    color: Constants.textSecondary
                    font.pixelSize: Constants.fontBody
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Adapter selection ──────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10
                visible: bluetoothHandler.adapterCount > 0

                Text {
                    text: "Adapter:"
                    color: Constants.textSecondary
                    font.pixelSize: Constants.fontLabel
                    anchors.verticalCenter: parent.verticalCenter
                }

                ComboBox {
                    id: adapterCombo
                    width: parent.width - 80
                    property var adapterList: bluetoothHandler.bluetoothAdapterList
                    model: adapterList
                    textRole: "name"

                    Component.onCompleted: {
                        if (!adapterList || adapterList.length === 0) return
                        var savedAddr = bluetoothHandler.getAdapterAddress()
                        for (var i = 0; i < adapterList.length; i++) {
                            if (adapterList[i].address === savedAddr) {
                                currentIndex = i
                                break
                            }
                        }
                    }

                    onActivated: {
                        if (currentIndex >= 0 && currentIndex < adapterList.length)
                            bluetoothHandler.setActiveAdapter(adapterList[currentIndex].address)
                    }
                }
            }

            // ── Paired devices ────────────────────────────────────────────────
            Text {
                text: "Paired Devices"
                color: Constants.textSecondary
                font.pixelSize: Constants.fontLabel
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

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Text {
                            text: model.name || model.address
                            color: Constants.textPrimary
                            font.pixelSize: Constants.fontBody
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            visible: model.connected
                            text: "(Connected)"
                            color: Constants.statusOk
                            font.pixelSize: Constants.fontCaption
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        JourneyButton {
                            text: model.connected ? "Disconnect" : "Connect"
                            icon.source: model.connected ? "images/link-off.svg" : "images/link.svg"
                            onClicked: {
                                if (model.connected)
                                    bluetoothHandler.disconnectDevice(model.address)
                                else
                                    bluetoothHandler.connectToDevice(model.address)
                            }
                        }

                        JourneyButton {
                            text: "Forget"
                            icon.source: "images/trash.svg"
                            onClicked: {
                                removeConfirmOverlay.deviceAddress = model.address
                                removeConfirmOverlay.deviceName = model.name || model.address
                                removeConfirmOverlay.visible = true
                            }
                        }
                    }
                }

                // Empty state
                Text {
                    visible: pairedList.count === 0
                    anchors.centerIn: parent
                    text: "No paired devices"
                    color: Constants.textDisabled
                    font.pixelSize: Constants.fontLabel
                }
            }

            // ── Scan controls ─────────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 12

                JourneyButton {
                    text: bluetoothHandler.isScanning ? "Scanning…" : "Scan for Devices"
                    icon.source: "images/search.svg"
                    enabled: !bluetoothHandler.isScanning
                    onClicked: bluetoothHandler.startScan()
                }

                Text {
                    visible: bluetoothHandler.isScanning
                    text: "Searching for devices…"
                    color: Constants.statusWait
                    font.pixelSize: Constants.fontLabel
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Pairing mode toggle ────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Pairing Mode"
                    color: Constants.textSecondary
                    font.pixelSize: Constants.fontLabel
                    anchors.verticalCenter: parent.verticalCenter
                }

                Switch {
                    id: pairingModeSwitch
                    checked: bluetoothHandler.isPairingModeEnabled
                    onCheckedChanged: bluetoothHandler.enablePairingMode(checked)
                }

                Text {
                    text: pairingModeSwitch.checked ? "On — device is discoverable" : "Off"
                    color: pairingModeSwitch.checked ? Constants.statusOk : Constants.textDisabled
                    font.pixelSize: Constants.fontCaption
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Available / scanned devices ───────────────────────────────────
            Text {
                visible: availableList.count > 0 || bluetoothHandler.isScanning
                text: "Available Devices"
                color: Constants.textSecondary
                font.pixelSize: Constants.fontLabel
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
                        color: Constants.textPrimary
                        font.pixelSize: Constants.fontBody
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        JourneyButton {
                            text: "Pair"
                            icon.source: "images/link.svg"
                            onClicked: bluetoothHandler.pair(model.address)
                        }

                        JourneyButton {
                            text: "Ignore"
                            icon.source: "images/close.svg"
                            onClicked: bluetoothHandler.ignoreDevice(model.address)
                        }
                    }
                }
            }
        }
    }
}