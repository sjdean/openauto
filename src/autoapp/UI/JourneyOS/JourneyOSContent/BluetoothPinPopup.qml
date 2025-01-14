import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 800
    height: 480

    // TODO: Bluetooth Status, Connectivity (Device), respond to Connectivity Request
    // TODO: Bluetooth PIN
    // TODO: Virtual Keyboard, Bluetooth Name

    Row {
        id: row
        spacing: 10
        padding: 10
        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10
            Text {
                text: "Unpaired"
                font.pointSize: 14
                font.bold: true
                color: "#ff990000"
            }
            Text {
                text: "Paired (Not Connected)"
                font.bold: true
                font.pointSize: 14
                color: "#ff999900"
            }
            Text {
                text: "Connecting..."
                font.bold: true
                font.pointSize: 14
                color: "#ff0000ff"
            }
            Text {
                text: "Connected"
                font.bold: true
                font.pointSize: 14
                color: "#ff009900"
            }
        }
        JourneyButton {
            id: journeyButton
            x: 198
            y: 45
            text: "Pair"
            anchors.verticalCenter: parent.verticalCenter
            icon.source: "images/fi-br-link.svg"
            iconSize: 24
        }
    }

}
