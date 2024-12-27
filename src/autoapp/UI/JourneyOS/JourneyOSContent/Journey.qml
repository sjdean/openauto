import QtQuick
import QtQuick.Controls
import JourneyOS 1.0

Window {
    id: root
    width: 800
    height: 480

    visible: true
    color: "#000000"                    // TODO: Change This
    title: "JourneyOS"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "MainView.qml"
    }

    Connections {
        target: stackView.currentItem
        function onViewSettings() {
            stackView.push("SettingsView.qml")
        }
    }
}
