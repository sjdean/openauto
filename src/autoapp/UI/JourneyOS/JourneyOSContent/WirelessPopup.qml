import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: wirelessPopup
    implicitWidth: 460
    implicitHeight: contentColumn.implicitHeight + 40

    signal close

    Rectangle {
        color: Constants.popupBackgroundTranslucent
        anchors.fill: parent
        radius: Constants.radiusCard
    }

    // ── Close button ──────────────────────────────────────────────────────────
    Rectangle {
        id: closeBtn
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 28
        height: 28
        radius: Constants.radiusCircle
        color: closeBtnArea.pressed ? Constants.btnDangerBgPressed : Constants.btnDangerBg
        z: 10

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
            onClicked: wirelessPopup.close()
        }
    }

    Column {
        id: contentColumn
        spacing: 8
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        anchors.topMargin: 10

        // ── Title ─────────────────────────────────────────────────────────────
        Text {
            text: "Wi-Fi"
            font.pixelSize: Constants.fontSubtitle
            font.bold: true
            color: Constants.textPrimary
        }

        // ── Status strip ──────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: statusRow.implicitHeight + 12
            color: wifiViewModel.connected ? Constants.statusBgOk : Constants.statusBgBad
            radius: Constants.radiusInput

            Row {
                id: statusRow
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: wifiViewModel.connected ? "● Connected" : "○ Not Connected"
                    color: wifiViewModel.connected ? Constants.statusOk : Constants.statusBad
                    font.pixelSize: Constants.fontCaption
                }
                Text {
                    visible: wifiViewModel.connected
                    text: wifiViewModel.currentSsid.length > 0 ? ("  " + wifiViewModel.currentSsid) : ""
                    color: Constants.textPrimary
                    font.pixelSize: Constants.fontCaption
                }
                Text {
                    visible: wifiViewModel.connected && wifiViewModel.signalStrength > 0
                    text: wifiViewModel.signalStrength + "%"
                    color: Constants.textPrimary
                    font.pixelSize: Constants.fontCaption
                }
            }
        }

        // ── Enable toggle ─────────────────────────────────────────────────────
        CheckBox {
            id: checkBoxEnableWireless
            text: qsTr("Enable Wireless")
            checked: wifiViewModel.isEnabled
            onCheckedChanged: wifiViewModel.isEnabled = checked
        }

        // ── Interface selection ───────────────────────────────────────────────
        Text { text: "Interface"; color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
        ComboBox {
            id: networkInterface
            width: parent.width
            model: networkAdapterModel.comboBoxItems
            textRole: "displayName"

            Component.onCompleted: {
                var items = networkAdapterModel.comboBoxItems
                for (var i = 0; i < items.length; i++) {
                    if (items[i].interfaceName === wifiViewModel.selectedInterface) {
                        currentIndex = i
                        break
                    }
                }
            }

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

            ButtonGroup { id: wirelessModeGroup }

            Row {
                spacing: 10
                RadioButton {
                    id: hotspotButton
                    text: qsTr("Hotspot")
                    checked: wifiViewModel.mode === 0
                    onCheckedChanged: if (checked) wifiViewModel.mode = 0
                    ButtonGroup.group: wirelessModeGroup
                }
                RadioButton {
                    id: clientButton
                    text: qsTr("Client")
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

                Text { text: qsTr("Hotspot SSID"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("SSID")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    text: wifiViewModel.hotspotSsid
                    onTextEdited: wifiViewModel.hotspotSsid = text
                }

                Text { text: qsTr("Hotspot Password"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("Password (min 8 chars)")
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
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

                Text { text: qsTr("Network SSID"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                TextField {
                    id: clientSsidField
                    width: parent.width
                    placeholderText: qsTr("Network name")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    text: wifiViewModel.clientSsid
                    onTextEdited: wifiViewModel.clientSsid = text
                }

                Text { text: qsTr("Password"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                Row {
                    width: parent.width
                    spacing: 8
                    TextField {
                        id: clientPasswordField
                        width: parent.width - connectBtn.width - disconnectBtn.width - 16
                        placeholderText: qsTr("Password")
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
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
                        color: Constants.textSecondary
                        font.pixelSize: Constants.fontSmall
                        font.italic: true
                    }
                }

                // Scan results list
                Rectangle {
                    visible: wifiViewModel.accessPoints.length > 0
                    width: parent.width
                    height: Math.min(apList.contentHeight + 2, 160)
                    color: Constants.overlaySubtle
                    radius: Constants.radiusInput
                    clip: true

                    ListView {
                        id: apList
                        anchors.fill: parent
                        model: wifiViewModel.accessPoints
                        spacing: 2

                        delegate: Rectangle {
                            width: apList.width
                            height: 36
                            color: apMouseArea.containsPress ? Constants.overlayPress : "transparent"
                            radius: Constants.radiusInput - 1

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 8

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.secured ? "\uD83D\uDD12" : "\uD83D\uDCF6"
                                    font.pixelSize: Constants.fontBody
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.ssid
                                    color: Constants.textPrimary
                                    font.pixelSize: Constants.fontLabel
                                    elide: Text.ElideRight
                                    width: parent.width - 80
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.strength + "%"
                                    color: modelData.strength > 60 ? Constants.statusOk
                                         : modelData.strength > 30 ? Constants.statusWait
                                         : Constants.statusBad
                                    font.pixelSize: Constants.fontCaption
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
            color: Constants.textDisabled
            font.italic: true
            wrapMode: Text.WordWrap
            width: parent.width
            font.pixelSize: Constants.fontCaption
        }

        Item { width: 1; height: 4 } // bottom padding
    }
}