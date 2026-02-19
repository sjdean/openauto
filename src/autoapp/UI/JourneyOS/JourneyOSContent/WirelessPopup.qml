import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: wirelessPopup
    implicitWidth: layoutColumn.implicitWidth + 40
    implicitHeight: layoutColumn.implicitHeight + 40

    Rectangle {
        color: Constants.settingsPopupBackgroundColor
        anchors.fill: parent
    }

    RoundButton {
        id: closeBtn
        text: "X"
        flat: true
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 5
        onClicked: wirelessPopup.parent.close()
    }

    Column {
        id: layoutColumn
        spacing: 10
        anchors.centerIn: parent
        width: parent.width - 20

        // ── Enable toggle — always visible ──────────────────────────────────
        CheckBox {
            id: checkBoxEnableWireless
            text: qsTr("Enable Wireless")
            checked: wifiViewModel.isEnabled
            onCheckedChanged: wifiViewModel.isEnabled = checked
        }

        // ── Interface selection — always visible ─────────────────────────────
        Text { text: "Interface" }
        ComboBox {
            id: networkInterface
            width: parent.width
            model: networkAdapterModel.comboBoxItems
            textRole: "displayName"
            currentIndex: {
                let idx = networkAdapterModel.comboBoxItems.findIndex(
                    item => item.address === wifiViewModel.selectedInterface)
                return idx !== -1 ? idx : 0
            }
            onCurrentIndexChanged: {
                if (currentIndex >= 0 && currentIndex < networkAdapterModel.comboBoxItems.length) {
                    wifiViewModel.selectedInterface = networkAdapterModel.comboBoxItems[currentIndex].address
                    networkAdapterModel.currentComboBoxItem = networkAdapterModel.comboBoxItems[currentIndex]
                }
            }
        }

        // ── Head Unit controls — hidden on Mac/Windows/Linux Desktop ─────────
        Column {
            visible: settingsViewHandler.headUnitMode
            width: parent.width
            spacing: 10

            ButtonGroup { id: wirelessModeGroup }

            Row {
                spacing: 10
                RadioButton {
                    id: hotspotButton
                    text: qsTr("Hotspot")
                    checked: wifiViewModel.mode === 1
                    onCheckedChanged: if (checked) wifiViewModel.mode = 1
                    ButtonGroup.group: wirelessModeGroup
                }
                RadioButton {
                    id: clientButton
                    text: qsTr("Client")
                    checked: wifiViewModel.mode === 2
                    onCheckedChanged: if (checked) wifiViewModel.mode = 2
                    ButtonGroup.group: wirelessModeGroup
                }
            }

            // Hotspot credentials
            Column {
                visible: wifiViewModel.mode === 1
                width: parent.width
                spacing: 6

                Text { text: qsTr("Hotspot SSID") }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("SSID")
                    text: wifiViewModel.hotspotSsid
                    onTextEdited: wifiViewModel.hotspotSsid = text
                }

                Text { text: qsTr("Hotspot Password") }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("Password")
                    echoMode: TextInput.Password
                    text: wifiViewModel.hotspotPassword
                    onTextEdited: wifiViewModel.hotspotPassword = text
                }
            }

            // Client credentials
            Column {
                visible: wifiViewModel.mode === 2
                width: parent.width
                spacing: 6

                Text { text: qsTr("Network SSID") }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("SSID")
                    text: wifiViewModel.currentSsid
                }

                Text { text: qsTr("Network Password") }
                TextField {
                    width: parent.width
                    placeholderText: qsTr("Password")
                    echoMode: TextInput.Password
                }
            }
        }

        // ── System-managed notice ────────────────────────────────────────────
        Text {
            visible: !settingsViewHandler.headUnitMode
            text: qsTr("Wi-Fi is managed by the operating system.\nEnable/disable and interface selection are available.")
            color: "gray"
            font.italic: true
            wrapMode: Text.WordWrap
            width: parent.width
        }
    }
}