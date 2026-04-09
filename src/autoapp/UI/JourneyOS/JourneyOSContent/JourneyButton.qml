import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects
import JourneyOS

Button {
    id: control
    property color textColor: Constants.btnContent
    property color iconColor: Constants.btnContent

    property double iconSize: 48
    property double buttonPadding: iconSize / 10
    property bool showText: true
    property bool textIsStatus: false

    visible: true

    leftPadding: buttonPadding
    rightPadding: buttonPadding

    implicitWidth: (control.textIsStatus ? horizontalButton.width : itemButton.width) + (buttonPadding * 2)
    implicitHeight: (control.textIsStatus ? horizontalButton.height : itemButton.height) + (buttonPadding * 2)

    text: "Button"
    font.pointSize: textIsStatus ? iconSize / 1.3 : (iconSize / 10) * 3
    icon.height: iconSize
    icon.width: iconSize
    icon.source: "images/settings.svg"
    display: AbstractButton.TextUnderIcon
    padding: buttonPadding
    bottomPadding: buttonPadding
    topPadding: buttonPadding

    clip: true
    icon.color: iconColor
    background: rectangleBackground

    Rectangle {
        id: rectangleBackground
        color: Constants.btnSurface
        opacity: enabled ? 1 : 0.3
        radius: Constants.shapeButton
        border.width: 0
        anchors.fill: parent
    }

    contentItem: control.textIsStatus ? horizontalButton : itemButton

    Item {
        id: itemButton
        anchors.centerIn: parent
        width: control.icon.width * 2
        height: control.icon.width * 2
        visible: !control.textIsStatus

        Column {
            id: column
            opacity: 1
            width: parent.width
            height: childrenRect.height
            spacing: control.buttonPadding
            padding: control.buttonPadding
            anchors.verticalCenter: itemButton.verticalCenter
            anchors.horizontalCenter: itemButton.horizontalCenter


            // Image + MultiEffect share the same Item so they overlap.
            // The Image must be visible (declared first = painted bottom) for
            // MultiEffect to have a texture to sample. MultiEffect is declared
            // second (painted on top) and replaces the source colours entirely.
            Item {
                width: control.icon.width
                height: control.icon.height
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: image
                    source: control.icon.source
                    anchors.fill: parent
                }

                MultiEffect {
                    source: image
                    anchors.fill: parent
                    brightness: 1.0
                }
            }

            Text {
                id: textItem
                text: control.text
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: enabled ? 1.0 : 0.3
                color: control.textColor
                font.pointSize: control.font.pointSize
                font.bold: true
                visible: control.text.length > 0
            }

        }
    }

    Item {
        id: horizontalButton
        anchors.centerIn: parent
        height: control.icon.height * 2
        width: childrenRect.width
        visible: control.textIsStatus

        Row {
            id: horizontalButtonRow
            height: parent.height
            spacing: control.buttonPadding

            Item {
                id: horizontalIcon
                anchors.verticalCenter: parent.verticalCenter
                height: (control.icon.height * 2) - (control.buttonPadding * 2)
                width: (control.icon.width * 2) - (control.buttonPadding * 2)

                // Image first (bottom), MultiEffect second (top) so the effect
                // paints over the original and colourises it correctly.
                Image {
                    id: horizontalIconImage
                    source: control.icon.source
                    height: control.icon.height
                    width: control.icon.width
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                MultiEffect {
                    source: horizontalIconImage
                    height: control.icon.height
                    width: control.icon.width
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    brightness: 1.0
                }
            }

            Item {
                id: horizontalTextItem
                anchors.verticalCenter: parent.verticalCenter
                visible: control.text.length > 0
                height: childrenRect.height
                width: childrenRect.width

                Text {
                    id: horizontalText
                    text: control.text
                    rightPadding: 5
                    leftPadding: 5
                    opacity: enabled ? 1.0 : 0.3
                    color: control.textColor
                    font.pointSize: control.font.pointSize
                    font.bold: true
                    visible: control.text.length > 0
                }
            }
        }
    }

    states: [
        State {
            name: "normal"
            when: !control.down

            PropertyChanges {
                target: rectangleBackground
                color: Constants.btnSurface
            }

            PropertyChanges {
                target: control
                icon.color: control.iconColor
            }

            PropertyChanges {
                target: textItem
                color: Constants.btnContent
            }
        },
        State {
            name: "down"
            when: control.down
            PropertyChanges {
                target: textItem
                color: Constants.btnContentPressed
            }

            PropertyChanges {
                target: control
                icon.color: Constants.btnContentPressed
            }

            PropertyChanges {
                target: rectangleBackground
                color: Constants.btnSurfacePressed
            }
        }
    ]
}
