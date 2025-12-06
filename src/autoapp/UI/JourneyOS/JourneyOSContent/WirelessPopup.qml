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

        // Anchor to top-right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 5

        // Logic to close/hide the popup
        onClicked: {
            wirelessPopup.parent.close()
        }
    }

    Column {
        id: layoutColumn
        spacing: 10
        anchors.centerIn: parent
        width: parent.width - 20

        CheckBox {
            id: checkBoxEnableWireless
            text: qsTr("Enable Wireless?")
            checked: wifiViewModel.isEnabled
            onCheckedChanged: wifiViewModel.isEnabled = checked
        }

        Text { text: "Interface" }
        ComboBox {
            id: networkInterface
            model: networkAdapterModel.comboBoxItems
            textRole: "displayName"
            currentIndex: {
                let index = networkAdapterModel.comboBoxItems.findIndex(item => item.address === wifiViewModel.selectedInterface);
                return index !== -1 ? index : 0; // Default to first item if not found
            }
            onCurrentIndexChanged: {
                if (currentIndex >= 0 && currentIndex < networkAdapterModel.comboBoxItems.length) {
                    wifiViewModel.selectedInterface = networkAdapterModel.comboBoxItems[currentIndex].address;
                    networkAdapterModel.currentComboBoxItem = networkAdapterModel.comboBoxItems[currentIndex];
                }
            }
        }

        TextField {
            id: textWirelessHotspotSSID
            placeholderText: qsTr("SSID")
            text: wifiViewModel.hotspotSsid
            onTextEdited: wifiViewModel.hotspotSsid = text
        }

        TextField {
            id: textWirelessHotspotPassword
            placeholderText: qsTr("Password")
            text: wifiViewModel.hotspotPassword
            onTextEdited: wifiViewModel.hotspotPassword = text
        }

        ButtonGroup {
            id: wirelessModeGroup
        }

        Row {
            spacing: 10
            width: implicitWidth
            RadioButton {
                id: hotspotButton
                text: qsTr("Hotspot")
                font.family: "Verdana"
                checked: wifiViewModel.mode === 1
                onCheckedChanged: wifiViewModel.mode = 1
                ButtonGroup.group: wirelessModeGroup
            }

            RadioButton {
                id: clientButton
                text: qsTr("Client")
                font.family: "Verdana"
                checked: settingsViewHandler.mode === 2
                onCheckedChanged: settingsViewHandler.mode = 2
                ButtonGroup.group: wirelessModeGroup
            }
        }
    }
}
