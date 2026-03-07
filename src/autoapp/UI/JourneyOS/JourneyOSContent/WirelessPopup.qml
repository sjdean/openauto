import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: wirelessPopup
    implicitWidth: 460
    implicitHeight: contentColumn.implicitHeight + 40

    Rectangle {
        color: Constants.settingsPopupBackgroundColor
        anchors.fill: parent
        radius: 8
    }

    RoundButton {
        id: closeBtn
        text: "X"
        flat: true
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 5
        z: 10
        onClicked: wirelessPopup.parent.close()
    }

    Column {
        id: contentColumn
        spacing: 8
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        anchors.topMargin: 10

        // ── Title ────────────────────────────────────────────────────────────
        Text {
            text: "Wi-Fi"
            font.pixelSize: 16
            font.bold: true
            color: Constants.primaryTextColor
        }

        // ── Status strip ─────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: statusRow.implicitHeight + 12
            color: wifiViewModel.connected ? "#22009900" : "#22990000"
            radius: 4

            Row {
                id: statusRow
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: wifiViewModel.connected ? "● Connected" : "○ Not Connected"
                    color: wifiViewModel.connected ? Constants.okColor : Constants.badColor
                    font.pixelSize: 12
                }
                Text {
                    visible: wifiViewModel.connected
                    text: wifiViewModel.currentSsid.length > 0 ? ("  " + wifiViewModel.currentSsid) : ""
                    color: Constants.primaryTextColor
                    font.pixelSize: 12
                }
                Text {
                    visible: wifiViewModel.connected && wifiViewModel.signalStrength > 0
                    text: wifiViewModel.signalStrength + "%"
                    color: Constants.primaryTextColor
                    font.pixelSize: 12
                }
            }
        }

        // ── Enable toggle ────────────────────────────────────────────────────
        CheckBox {
            id: checkBoxEnableWireless
            text: qsTr("Enable Wireless")
            checked: wifiViewModel.isEnabled
            onCheckedChanged: wifiViewModel.isEnabled = checked
        }

        // ── Interface selection ───────────────────────────────────────────────
        Text { text: "Interface"; color: Constants.primaryTextColor; font.pixelSize: 12 }
        ComboBox {
            id: networkInterface
            width: parent.width
            model: networkAdapterModel.comboBoxItems
            textRole: "displayName"

            // Set initial selection by interface name (the value WifiController needs).
            Component.onCompleted: {
                var items = networkAdapterModel.comboBoxItems
                for (var i = 0; i < items.length; i++) {
                    if (items[i].interfaceName === wifiViewModel.selectedInterface) {
                        currentIndex = i
                        break
                    }
                }
            }

            // onActivated fires only on explicit user interaction, not on model load.
            onActivated: {
                var items = networkAdapterModel.comboBoxItems
                if (currentIndex >= 0 && currentIndex < items.length) {
                    wifiViewModel.selectedInterface = items[currentIndex].interfaceName
                    networkAdapterModel.currentComboBoxItem = items[currentIndex]
                }
            }
        }

        // ── Head Unit controls ────────────────────────────────────────────────
        Column {
            visible: ConfigGate.showConfig
            width: parent.width
            spacing: 8

            // Mode selector
            ButtonGroup { id: wirelessModeGroup }

            Row {
                spacing: 10
                RadioButton {
                    id: hotspotButton
                    text: qsTr("Hotspot")
                    // WIRELESS_HOTSPOT = 0
                    checked: wifiViewModel.mode === 0
                    onCheckedChanged: if (checked) wifiViewModel.mode = 0
                    ButtonGroup.group: wirelessModeGroup
                }
                RadioButton {
                    id: clientButton
                    text: qsTr("Client")
                    // WIRELESS_CLIENT = 1
                    checked: wifiViewModel.mode === 1
                    onCheckedChanged: if (checked) wifiViewModel.mode = 1
                    ButtonGroup.group: wirelessModeGroup
                }
            }

            // ── Hotspot credentials ───────────────────────────────────────────
            Column {
                visible: wifiViewModel.mode === 0
                width: parent.width
                spacing: 6

                Text { text: qsTr("Hotspot SSID"); color: Constants.primaryTextColor; font.pixelSize: 12 }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("SSID")
                    text: wifiViewModel.hotspotSsid
                    onTextEdited: wifiViewModel.hotspotSsid = text
                }

                Text { text: qsTr("Hotspot Password"); color: Constants.primaryTextColor; font.pixelSize: 12 }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("Password (min 8 chars)")
                    echoMode: TextInput.Password
                    text: wifiViewModel.hotspotPassword
                    onTextEdited: wifiViewModel.hotspotPassword = text
                }

                Row {
                    spacing: 8
                    Button {
                        text: "Start Hotspot"
                        onClicked: wifiViewModel.applyHotspot()
                    }
                    Button {
                        text: "Stop"
                        onClicked: wifiViewModel.disconnectCurrent()
                    }
                }
            }

            // ── Client credentials + scan ─────────────────────────────────────
            Column {
                visible: wifiViewModel.mode === 1
                width: parent.width
                spacing: 6

                // Saved credentials row
                Text { text: qsTr("Network SSID"); color: Constants.primaryTextColor; font.pixelSize: 12 }
                TextField {
                    id: clientSsidField
                    width: parent.width
                    placeholderText: qsTr("Network name")
                    text: wifiViewModel.clientSsid
                    onTextEdited: wifiViewModel.clientSsid = text
                }

                Text { text: qsTr("Password"); color: Constants.primaryTextColor; font.pixelSize: 12 }
                Row {
                    width: parent.width
                    spacing: 8
                    TextField {
                        id: clientPasswordField
                        width: parent.width - connectBtn.width - disconnectBtn.width - 16
                        placeholderText: qsTr("Password")
                        echoMode: TextInput.Password
                        text: wifiViewModel.clientPassword
                        onTextEdited: wifiViewModel.clientPassword = text
                    }
                    Button {
                        id: connectBtn
                        text: "Connect"
                        onClicked: wifiViewModel.connectToNetwork(clientSsidField.text, clientPasswordField.text)
                    }
                    Button {
                        id: disconnectBtn
                        text: "Disconnect"
                        enabled: wifiViewModel.connected
                        onClicked: wifiViewModel.disconnectCurrent()
                    }
                }

                // Scan controls
                Row {
                    width: parent.width
                    spacing: 8
                    Button {
                        text: "Scan"
                        onClicked: wifiViewModel.doWirelessNetworkScan()
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: wifiViewModel.accessPoints.length > 0
                              ? (wifiViewModel.accessPoints.length + " network(s) found")
                              : "No scan results"
                        color: Constants.primaryTextColor
                        font.pixelSize: 11
                        font.italic: true
                    }
                }

                // Scan results list
                Rectangle {
                    visible: wifiViewModel.accessPoints.length > 0
                    width: parent.width
                    height: Math.min(apList.contentHeight + 2, 160)
                    color: "#22ffffff"
                    radius: 4
                    clip: true

                    ListView {
                        id: apList
                        anchors.fill: parent
                        model: wifiViewModel.accessPoints
                        spacing: 2

                        delegate: Rectangle {
                            width: apList.width
                            height: 36
                            color: apMouseArea.containsPress ? "#44ffffff" : "transparent"
                            radius: 3

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 8

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.secured ? "🔒" : "📶"
                                    font.pixelSize: 14
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.ssid
                                    color: Constants.primaryTextColor
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    width: parent.width - 80
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.strength + "%"
                                    color: modelData.strength > 60 ? Constants.okColor
                                         : modelData.strength > 30 ? Constants.waitColor
                                         : Constants.badColor
                                    font.pixelSize: 12
                                }
                            }

                            MouseArea {
                                id: apMouseArea
                                anchors.fill: parent
                                onClicked: {
                                    clientSsidField.text = modelData.ssid
                                    wifiViewModel.clientSsid = modelData.ssid
                                    clientPasswordField.text = ""
                                    clientPasswordField.forceActiveFocus()
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── System-managed notice ─────────────────────────────────────────────
        Text {
            visible: !ConfigGate.showConfig
            text: qsTr("Wi-Fi is managed by the operating system.\nInterface selection is available.")
            color: "gray"
            font.italic: true
            wrapMode: Text.WordWrap
            width: parent.width
            font.pixelSize: 12
        }

        Item { width: 1; height: 4 } // bottom padding
    }
}