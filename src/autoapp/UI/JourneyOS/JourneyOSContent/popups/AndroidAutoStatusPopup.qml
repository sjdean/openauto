import QtQuick
import QtQuick.Controls
import JourneyOS
import AndroidAutoMonitor

Popup {
    id: aaInfoPopup
    anchors.centerIn: parent
    width: 300
    height: androidAutoConnected ? 210 : 175
    modal: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal viewAndroidAutoRequested()

    property bool androidAutoConnected: false
    property bool androidAutoConnecting: false
    property string androidAutoMethodText: ""

    background: Rectangle {
        color: Constants.popupBackgroundTranslucent
        radius: Constants.radiusPopup
        border.color: Constants.popupBorder
        border.width: 1
    }

    // Close ✕ button — top-right corner
    Rectangle {
        id: aaCloseBtn
        width: 28; height: 28
        radius: Constants.radiusCircle
        color: aaCloseArea.pressed ? Constants.btnDangerBgPressed : Constants.btnDangerBg
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 8
        anchors.rightMargin: 8
        z: 1
        Behavior on color { ColorAnimation { duration: 60 } }
        Text {
            anchors.centerIn: parent
            text: "\u2715"
            font.pixelSize: 14
            font.bold: true
            color: Constants.btnDangerFg
        }
        MouseArea {
            id: aaCloseArea
            anchors.fill: parent
            onClicked: aaInfoPopup.close()
        }
    }

    Column {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 10   // shift below the close button
        spacing: 14

        Text {
            text: "Android Auto"
            font.pixelSize: Constants.fontHeading
            font.bold: true
            color: Constants.textPrimary
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: {
                if (aaInfoPopup.androidAutoConnected)
                    "Connected via " + aaInfoPopup.androidAutoMethodText
                else if (aaInfoPopup.androidAutoConnecting)
                    "Connecting\u2026"
                else
                    "Not Connected"
            }
            font.pixelSize: Constants.fontBody
            color: Constants.textSecondary
            anchors.horizontalCenter: parent.horizontalCenter
        }

        JourneyButton {
            text: "View Android Auto"
            icon.source: "../images/android-auto.svg"
            iconSize: 20
            visible: aaInfoPopup.androidAutoConnected
            width: 200
            height: 40
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                aaInfoPopup.close()
                aaInfoPopup.viewAndroidAutoRequested()
            }
        }
    }
}
