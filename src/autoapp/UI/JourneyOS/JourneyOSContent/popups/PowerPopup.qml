import QtQuick
import QtQuick.Controls
import JourneyOS

Popup {
    id: powerPopup
    anchors.centerIn: parent
    width: 420
    height: 220
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal rebootRequested()
    signal shutdownRequested()

    background: Rectangle {
        color: Constants.popupBackgroundTranslucent
        radius: Constants.radiusPopup
        border.color: Constants.popupBorder
        border.width: 1
    }

    Column {
        anchors.centerIn: parent
        spacing: 16

        Text {
            text: "System Power"
            font.pixelSize: Constants.fontTitle
            font.bold: true
            color: Constants.textPrimary
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Choose an action or cancel."
            font.pixelSize: Constants.fontBody
            color: Constants.textSecondary
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 12
            anchors.horizontalCenter: parent.horizontalCenter

            // Cancel — red outline, closes popup without action
            Button {
                text: "Cancel"
                width: 110; height: 48
                background: Rectangle {
                    color: "transparent"
                    radius: Constants.radiusButton
                    border.color: Constants.btnCancelBorder
                    border.width: 2
                    opacity: parent.down ? 0.6 : 1.0
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: Constants.fontBody
                    font.bold: true
                    color: Constants.btnCancelFg
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: parent.parent.down ? 0.6 : 1.0
                }
                onClicked: powerPopup.close()
            }

            // Reboot — deep blue, safe action (system restarts)
            Button {
                text: "Reboot"
                width: 110; height: 48
                background: Rectangle {
                    color: parent.down ? Constants.btnActionBgPressed : Constants.btnActionBg
                    radius: Constants.radiusButton
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: Constants.fontBody
                    font.bold: true
                    color: Constants.btnActionFg
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: powerPopup.rebootRequested()
            }

            // Shutdown — dark red, destructive action (system stops)
            Button {
                text: "Shutdown"
                width: 110; height: 48
                background: Rectangle {
                    color: parent.down ? Constants.btnDangerBgPressed : Constants.btnDangerBg
                    radius: Constants.radiusButton
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: Constants.fontBody
                    font.bold: true
                    color: Constants.btnDangerFg
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: powerPopup.shutdownRequested()
            }
        }
    }
}
