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

                        Button {
                            text: "Accept"
                            onClicked: {
                                if (bluetoothHandler.agent)
                                    bluetoothHandler.agent.accept()
                                pinOverlay.visible = false
                            }
                        }

                        Button {
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

                        Button {
                            text: "Forget"
                            onClicked: {
                                bluetoothHandler.removePair(removeConfirmOverlay.deviceAddress)
                                removeConfirmOverlay.visible = false
                            }
                        }

                        Button {
                            text: "Cancel"
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
            spacing: 10
            visible: ConfigGate.showConfig

            // ── Title + status ────────────────────────────────────────────────
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

                Text {
                    text: "Adapter:"
                    color: Constants.textSecondary
                    font.pixelSize: Constants.fontLabel
                    anchors.verticalCenter: parent.verticalCenter
                    visible: bluetoothHandler.adapterCount > 1
                }

                ComboBox {
                    id: adapterCombo
                    width: parent.width - 80
                    visible: bluetoothHandler.adapterCount > 1

                    // adapterList is a QVariantList of QVariantMap {name, address, powered, discoverable}
                    model: bluetoothHandler.bluetoothAdapterList
                    textRole: "name"

                    Component.onCompleted: refreshAdapterIndex()

                    function refreshAdapterIndex() {
                        var list = bluetoothHandler.bluetoothAdapterList
                        if (!list || list.length === 0) return
                        var savedAddr = bluetoothHandler.getAdapterAddress()
                        for (var i = 0; i < list.length; i++) {
                            if (list[i].address === savedAddr) {
                                currentIndex = i
                                return
                            }
                        }
                        currentIndex = 0
                    }

                    onActivated: {
                        var list = bluetoothHandler.bluetoothAdapterList
                        if (currentIndex >= 0 && currentIndex < list.length)
                            bluetoothHandler.setActiveAdapter(list[currentIndex].address)
                    }

                    Connections {
                        target: bluetoothHandler
                        function onBluetoothAdapterListChanged() { adapterCombo.refreshAdapterIndex() }
                    }
                }

                // When only one adapter — just show its name + address inline
                Text {
                    visible: bluetoothHandler.adapterCount <= 1
                    anchors.verticalCenter: parent.verticalCenter
                    color: Constants.textPrimary
                    font.pixelSize: Constants.fontLabel
                    text: {
                        var list = bluetoothHandler.bluetoothAdapterList
                        if (list && list.length > 0) return list[0].name
                        return "No adapter"
                    }
                }
            }

            // ── Local device info (address + discoverability) ─────────────────
            Rectangle {
                width: parent.width
                height: addrRow.implicitHeight + 10
                color: Constants.overlaySubtle
                radius: Constants.radiusInput

                Row {
                    id: addrRow
                    anchors.centerIn: parent
                    spacing: 20

                    Text {
                        text: "Address: " + bluetoothHandler.getAdapterAddress()
                        color: Constants.textSecondary
                        font.pixelSize: Constants.fontCaption
                        font.family: "monospace"
                    }

                    Text {
                        text: bluetoothHandler.isPairingModeEnabled ? "Discoverable" : "Connectable"
                        color: bluetoothHandler.isPairingModeEnabled ? Constants.statusOk : Constants.textDisabled
                        font.pixelSize: Constants.fontCaption
                    }
                }
            }

            // ── Pairing mode toggle ────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Discoverable:"
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
                    text: pairingModeSwitch.checked ? "On — device is visible to others" : "Off"
                    color: pairingModeSwitch.checked ? Constants.statusOk : Constants.textDisabled
                    font.pixelSize: Constants.fontCaption
                    anchors.verticalCenter: parent.verticalCenter
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
                height: 120
                clip: true
                model: bluetoothHandler.pairedDeviceList

                delegate: Rectangle {
                    width: pairedList.width
                    height: 44
                    color: "transparent"
                    // thin separator line below
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Constants.overlaySubtle
                    }

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Rectangle {
                            width: 8; height: 8; radius: 4
                            anchors.verticalCenter: parent.verticalCenter
                            color: modelData.connected ? Constants.statusOk : Constants.textDisabled
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: (modelData.name && modelData.name.length > 0) ? modelData.name : modelData.address
                                color: Constants.textPrimary
                                font.pixelSize: Constants.fontBody
                            }
                            Text {
                                visible: modelData.name && modelData.name.length > 0
                                text: modelData.address
                                color: Constants.textDisabled
                                font.pixelSize: Constants.fontCaption
                                font.family: "monospace"
                            }
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Button {
                            text: modelData.connected ? "Disconnect" : "Connect"
                            height: 32
                            onClicked: {
                                if (modelData.connected)
                                    bluetoothHandler.disconnectDevice(modelData.address)
                                else
                                    bluetoothHandler.connectToDevice(modelData.address)
                            }
                        }

                        Button {
                            text: "Forget"
                            height: 32
                            onClicked: {
                                removeConfirmOverlay.deviceAddress = modelData.address
                                removeConfirmOverlay.deviceName = (modelData.name && modelData.name.length > 0) ? modelData.name : modelData.address
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

                Button {
                    text: bluetoothHandler.isScanning ? "Scanning…" : "Scan for Devices"
                    enabled: !bluetoothHandler.isScanning
                    onClicked: bluetoothHandler.startScan()
                }

                Text {
                    visible: bluetoothHandler.isScanning
                    text: "Searching for nearby devices…"
                    color: Constants.statusWait
                    font.pixelSize: Constants.fontLabel
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // ── Available / scanned devices ───────────────────────────────────
            Text {
                visible: availableList.count > 0
                text: "Available Devices"
                color: Constants.textSecondary
                font.pixelSize: Constants.fontLabel
                font.bold: true
            }

            ListView {
                id: availableList
                width: parent.width
                height: 100
                clip: true
                visible: count > 0
                model: bluetoothHandler.unpairedDeviceList

                delegate: Rectangle {
                    width: availableList.width
                    height: 40
                    color: "transparent"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Constants.overlaySubtle
                    }

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: (modelData.name && modelData.name.length > 0) ? modelData.name : modelData.address
                                color: Constants.textPrimary
                                font.pixelSize: Constants.fontBody
                            }
                            Text {
                                text: modelData.address
                                color: Constants.textDisabled
                                font.pixelSize: Constants.fontCaption
                                font.family: "monospace"
                            }
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Button {
                            text: "Pair"
                            height: 32
                            onClicked: bluetoothHandler.pair(modelData.address)
                        }

                        Button {
                            text: "Ignore"
                            height: 32
                            onClicked: bluetoothHandler.ignoreDevice(modelData.address)
                        }
                    }
                }
            }
        }
    }
}