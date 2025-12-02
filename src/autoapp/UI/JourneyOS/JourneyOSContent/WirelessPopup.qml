import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: wirelessPopup

    // TODO: Add Button to call Randomizer for Wireless Password

    Rectangle {
        color: Constants.settingsPopupBackgroundColor
        anchors.fill: parent
    }

    Column {
        spacing: 10
        padding: 10
        width: implicitWidth
        height: implicitHeight

        CheckBox {
            id: checkBoxEnableWireless
            text: qsTr("Enable Wireless?")
            checked: settingsViewHandler.wirelessEnabled
            onCheckedChanged: settingsViewHandler.wirelessEnabled = checked
        }

        Text { text: "Interface" }
        ComboBox {
            id: interfaceCombo
            width: parent.width
            model: networkAdapterModel.comboBoxItems
            textRole: "displayName"

            // Set initial value
            Component.onCompleted: {
                var idx = networkAdapterModel.indexOf(wifiMonitor.interfaceName)
                if (idx >= 0) currentIndex = idx
            }

            onCurrentIndexChanged: {
                var item = model[currentIndex]
                // Tell the monitor to switch interfaces
                if(item) wifiMonitor.interfaceName = item.interfaceName
            }
        }

        TextField {
            id: textWirelessHotspotSSID
            placeholderText: qsTr("SSID")
            text: settingsViewHandler.wirelessHotspotSSID
            onTextEdited: settingsViewHandler.wirelessHotspotSSID = text
        }

        TextField {
            id: textWirelessHotspotPassword
            placeholderText: qsTr("Password")
            text: settingsViewHandler.wirelessHotspotPassword
            onTextEdited: settingsViewHandler.wirelessHotspotPassword = text
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
                checked: settingsViewHandler.wirelessType === 1
                onCheckedChanged: settingsViewHandler.wirelessType = 1
                ButtonGroup.group: wirelessModeGroup
            }

            RadioButton {
                id: clientButton
                text: qsTr("Client")
                font.family: "Verdana"
                checked: settingsViewHandler.wirelessType === 2
                onCheckedChanged: settingsViewHandler.wirelessType = 2
                ButtonGroup.group: wirelessModeGroup
            }
        }
    }
}
