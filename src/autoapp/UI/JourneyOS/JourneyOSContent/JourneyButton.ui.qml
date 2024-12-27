/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Button {
    id: control
    property color textColor: "#ffffff"
    property color iconColor: "#ffffff"
    property color backgroundColor: "#2d004d"

    property double iconSize: 48
    property double buttonPadding: iconSize / 10
    property bool showText: true
    property bool textIsStatus: false

    visible: true
    //implicitWidth: control.textIsStatus ? itemButtonRow.width : itemButton.width
    //implicitHeight: control.textIsStatus ? itemButtonRow.height : itemButton.height

   // implicitWidth: Math.max(
   //                    rectangleBackground ? rectangleBackground.implicitWidth : 0,
   //                    textItem.implicitWidth + leftPadding + rightPadding)
   // implicitHeight: Math.max(
   //                     rectangleBackground ? rectangleBackground.implicitHeight : 0,
   //                     textItem.implicitHeight + topPadding + bottomPadding)

    leftPadding: buttonPadding
    rightPadding: buttonPadding

    width: (control.textIsStatus ? horizontalButton.width : itemButton.width) + buttonPadding
    height: (control.textIsStatus ? horizontalButton.height : itemButton.height) + buttonPadding

    text: "Button"
    font.pointSize: textIsStatus ? iconSize / 1.3 : (iconSize / 10) * 3
    icon.height: iconSize
    icon.width: iconSize
    icon.source: "images/pen-clip.svg"
    display: AbstractButton.TextUnderIcon
    padding: buttonPadding
    bottomPadding: buttonPadding
    topPadding: buttonPadding

    clip: true
    icon.color: iconColor
    background: rectangleBackground

    Rectangle {
        id: rectangleBackground
        color: "#55999999"
        opacity: enabled ? 1 : 0.3
        radius: 5
        border.width: 0
        anchors.fill: parent

    }

    contentItem: control.textIsStatus ? horizontalButton : itemButton

    Item {
        id: itemButton
        x: control.buttonPadding
        y: control.buttonPadding
        width: control.icon.width * 2 - (control.buttonPadding * 2)
        height: control.icon.width * 2 - (control.buttonPadding * 2)
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


            ColorOverlay {
                id: colorOverlay
                source: image
                height: control.icon.height
                width: control.icon.width
                color: control.icon.color
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Image {
                id: image
                source: control.icon.source
                height: control.icon.height
                width: control.icon.width
                anchors.horizontalCenter: parent.horizontalCenter
                visible: false
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
        x: control.buttonPadding
        y: control.buttonPadding
        height: control.icon.height * 2 - (control.buttonPadding * 2)
        width: childrenRect.width
        visible: control.textIsStatus

        Row {
            id: horizontalButtonRow
            height: parent.height
            width: childrenRect.width
            spacing: control.buttonPadding

            Item {
                id: horizontalIcon
                anchors.verticalCenter: parent.verticalCenter
                height: (control.icon.height * 2) - (control.buttonPadding * 2)
                width: (control.icon.width * 2) - (control.buttonPadding * 2)

                ColorOverlay {
                    id: horizontalIconOverlay
                    source: horizontalIconImage
                    height: control.icon.height
                    width: control.icon.width
                    color: control.icon.color
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Image {
                    id: horizontalIconImage
                    source: control.icon.source
                    height: control.icon.height
                    width: control.icon.width
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: false
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
                target: buttonBackground
                color: "#55999999"
            }

            PropertyChanges {
                target: control
                icon.color: control.iconColor
            }

            PropertyChanges {
                target: textItem
                color: "#ffffff"
            }
        },
        State {
            name: "down"
            when: control.down
            PropertyChanges {
                target: textItem
                color: "#80ffffff"
            }

            PropertyChanges {
                target: control
                icon.color: "#80ffffff"
            }

            PropertyChanges {
                target: buttonBackground
                color: "#bb999999"
            }
        }

    ]
}
