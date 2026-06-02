// WirelessPopupPreview.qml
// ─────────────────────────────────────────────────────────────────────────────
// Design Studio / standalone preview.  Open via JourneyOS.qmlproject.
// All C++ context properties are replaced with local mock state.
// Toggle mockMode / mockConnected / mockScanDone to preview different states.
// ─────────────────────────────────────────────────────────────────────────────
import QtQuick
import QtQuick.Controls
import JourneyOS

Rectangle {
    id: preview
    width: 800
    height: 480
    color: "#222222"

    // ── Mock state — change these to preview different UI states ──────────────
    property bool mockConnected:   true
    property bool mockEnabled:     true
    property int  mockMode:        1        // 0 = Hotspot, 1 = Client
    property bool mockShowConfig:  true     // ConfigGate.showConfig equivalent
    property bool mockScanDone:    true     // false = no results yet
    property string mockSsid:      "HomeNetwork_5G"
    property string mockIp:        "192.168.1.42"
    property string mockMac:       "DC:A6:32:1B:4F:9E"
    property string mockScanStatus: mockScanDone ? "8 network(s) found" : ""

    ListModel {
        id: mockApModel
        ListElement { ssid: "HomeNetwork_5G";    strength: 91; secured: true  }
        ListElement { ssid: "BT-Hub6-3F2A";      strength: 78; secured: true  }
        ListElement { ssid: "SKY12345";           strength: 65; secured: true  }
        ListElement { ssid: "AndroidAP_Simon";    strength: 54; secured: false }
        ListElement { ssid: "Virgin Media Guest"; strength: 41; secured: false }
        ListElement { ssid: "TALKTALK-A3B2";      strength: 32; secured: true  }
        ListElement { ssid: "EE-BrightBox-7823";  strength: 18; secured: true  }
        ListElement { ssid: "xfinitywifi";         strength:  9; secured: false }
    }

    // ── Popup centred on the preview canvas ───────────────────────────────────
    Item {
        id: popupRoot
        anchors.centerIn: parent
        width: 460
        implicitHeight: scroller.height

        Rectangle {
            anchors.fill: parent
            color: Constants.popupBackgroundTranslucent
            radius: Constants.radiusCard
        }

        // Close button
        Rectangle {
            id: closeBtn
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            width: 28; height: 28
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
            MouseArea { id: closeBtnArea; anchors.fill: parent }
        }

        ScrollView {
            id: scroller
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: Math.min(contentColumn.implicitHeight, 420)
            contentWidth: availableWidth
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Column {
                id: contentColumn
                spacing: 8
                width: scroller.availableWidth - 40
                leftPadding: 20
                topPadding: 10
                bottomPadding: 10

                // Title
                Text {
                    text: "Wi-Fi"
                    font.pixelSize: Constants.fontSubtitle
                    font.bold: true
                    color: Constants.textPrimary
                }

                // Status strip
                Rectangle {
                    width: parent.width
                    height: statusCol.implicitHeight + 12
                    color: preview.mockConnected ? Constants.statusBgOk : Constants.statusBgBad
                    radius: Constants.radiusInput

                    Column {
                        id: statusCol
                        anchors.centerIn: parent
                        spacing: 4

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 12
                            Text {
                                text: preview.mockConnected ? "● Connected" : "○ Not Connected"
                                color: preview.mockConnected ? Constants.statusOk : Constants.statusBad
                                font.pixelSize: Constants.fontCaption
                            }
                            Text {
                                visible: preview.mockConnected
                                text: preview.mockSsid
                                color: Constants.textPrimary
                                font.pixelSize: Constants.fontCaption
                            }
                            Text {
                                visible: preview.mockConnected
                                text: "91%"
                                color: Constants.textPrimary
                                font.pixelSize: Constants.fontCaption
                            }
                        }
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 16
                            Text {
                                text: "MAC: " + preview.mockMac
                                color: Constants.textSecondary
                                font.pixelSize: Constants.fontCaption
                            }
                            Text {
                                visible: preview.mockConnected
                                text: "IP: " + preview.mockIp
                                color: Constants.textSecondary
                                font.pixelSize: Constants.fontCaption
                            }
                        }
                    }
                }

                // Enable toggle
                CheckBox {
                    text: qsTr("Enable Wireless")
                    checked: preview.mockEnabled
                }

                // Interface
                Text { text: "Interface"; color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                ComboBox {
                    width: parent.width
                    model: ["wlan0 (DC:A6:32:1B:4F:9E)"]
                }

                // Config section
                Column {
                    visible: preview.mockShowConfig
                    width: parent.width
                    spacing: 8

                    ButtonGroup { id: mockModeGroup }
                    Row {
                        spacing: 10
                        RadioButton {
                            text: qsTr("Hotspot")
                            checked: preview.mockMode === 0
                            onCheckedChanged: if (checked) preview.mockMode = 0
                            ButtonGroup.group: mockModeGroup
                        }
                        RadioButton {
                            text: qsTr("Client")
                            checked: preview.mockMode === 1
                            onCheckedChanged: if (checked) preview.mockMode = 1
                            ButtonGroup.group: mockModeGroup
                        }
                    }

                    // Hotspot section
                    Column {
                        visible: preview.mockMode === 0
                        width: parent.width
                        spacing: 6
                        Text { text: qsTr("Hotspot SSID"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                        TextField { width: parent.width; placeholderText: qsTr("SSID"); text: "JourneyOS-AP" }
                        Text { text: qsTr("Hotspot Password"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                        TextField { width: parent.width; placeholderText: qsTr("Password (min 8 chars)"); echoMode: TextInput.Password; text: "password1" }
                        Row {
                            spacing: 8
                            Button { text: "Start Hotspot" }
                            Button { text: "Stop" }
                        }
                    }

                    // Client section
                    Column {
                        visible: preview.mockMode === 1
                        width: parent.width
                        spacing: 6

                        Text { text: qsTr("Network SSID"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                        TextField {
                            id: mockSsidField
                            width: parent.width
                            placeholderText: qsTr("Network name")
                            text: preview.mockConnected ? preview.mockSsid : ""
                        }

                        Text { text: qsTr("Password"); color: Constants.textSecondary; font.pixelSize: Constants.fontCaption }
                        Row {
                            width: parent.width
                            spacing: 8
                            TextField {
                                id: mockPwField
                                width: parent.width - mockConnectBtn.width - mockDisconnectBtn.width - 16
                                placeholderText: qsTr("Password")
                                echoMode: TextInput.Password
                            }
                            Button { id: mockConnectBtn;    text: "Connect" }
                            Button { id: mockDisconnectBtn; text: "Disconnect"; enabled: preview.mockConnected }
                        }

                        Button {
                            text: "Scan"
                            onClicked: preview.mockScanDone = true
                        }

                        Text {
                            visible: preview.mockScanStatus.length > 0
                            width: parent.width
                            text: preview.mockScanStatus
                            color: Constants.textSecondary
                            font.pixelSize: Constants.fontSmall
                            font.italic: true
                        }

                        // AP list — 5 rows, scrollable
                        ListView {
                            id: mockApList
                            visible: preview.mockScanDone
                            width: parent.width
                            height: 5 * 40
                            clip: true
                            model: mockApModel
                            spacing: 2

                            delegate: Rectangle {
                                required property string ssid
                                required property int    strength
                                required property bool   secured
                                required property int    index

                                width: mockApList.width
                                height: 38
                                color: apArea.containsPress
                                       ? Constants.overlayPress
                                       : (index % 2 === 0 ? Constants.overlaySubtle : "transparent")
                                radius: Constants.radiusInput

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    spacing: 8

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: secured ? "\uD83D\uDD12" : "\uD83D\uDCF6"
                                        font.pixelSize: Constants.fontBody
                                    }
                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: ssid
                                        color: Constants.textPrimary
                                        font.pixelSize: Constants.fontLabel
                                        elide: Text.ElideRight
                                        width: parent.width - 80
                                    }
                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: strength + "%"
                                        color: strength > 60 ? Constants.statusOk
                                             : strength > 30 ? Constants.statusWait
                                             : Constants.statusBad
                                        font.pixelSize: Constants.fontCaption
                                    }
                                }

                                MouseArea {
                                    id: apArea
                                    anchors.fill: parent
                                    onClicked: {
                                        mockSsidField.text = ssid
                                        mockPwField.forceActiveFocus()
                                    }
                                }
                            }
                        }
                    }
                }

                // System-managed notice
                Text {
                    visible: !preview.mockShowConfig
                    text: qsTr("Wi-Fi is managed by the operating system.\nInterface selection is available.")
                    color: Constants.textDisabled
                    font.italic: true
                    wrapMode: Text.WordWrap
                    width: parent.width
                    font.pixelSize: Constants.fontCaption
                }
            }
        }
    }
}