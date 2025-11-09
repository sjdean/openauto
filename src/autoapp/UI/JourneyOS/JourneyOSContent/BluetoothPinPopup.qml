import QtQuick
import QtQuick.Controls
import JourneyOS

Column { // Use Column
    width: 300 // Set size on root
    height: 200
    spacing: 10
    anchors.margins: 10

    // This 'agent' property is passed in from MainView.qml
    property var agent

    // This 'pinText' property is set by MainView's Connections
    property string pinText: "Confirm pairing?"

    signal accepted
    signal rejected

    Text {
        text: pinText // Display the text from the agent
        wrapMode: Text.WordWrap
        width: parent.width // Manually set width
    }

    Row { // Use Row
        spacing: 10
        anchors.horizontalCenter: parent.horizontalCenter // Center the row

        JourneyButton {
            text: "Accept"
            onClicked: {
                agent.accept()
                accepted() // Tell MainView to close us
            }
        }
        JourneyButton {
            text: "Decline"
            onClicked: {
                agent.reject()
                rejected() // Tell MainView to close us
            }
        }
    }
}
