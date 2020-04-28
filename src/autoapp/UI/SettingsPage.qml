import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

ScrollablePage {
    id: settingsPage

    Column {
        spacing: 40
        width: parent.width

        Label {
            width: parent.width
            wrapMode: Label.wrapMode
            horizontalAlignment: Qt.AlignHCenter
            text: "some text";
        }

        PageIndicator {
            count: 5
            currentIndex: 2
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
