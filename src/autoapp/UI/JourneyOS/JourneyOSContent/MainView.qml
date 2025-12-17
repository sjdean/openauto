import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: mainView
    width: 800
    height: 480
    objectName: "MainView" // Useful for debugging

    // Signals to tell Journey.qml what to do
    signal viewSettings()
    signal viewVolume()
    signal viewBrightness()
    signal viewBluetoothStatus()
    signal viewWifiStatus()
    signal viewAndroidAuto()

    property bool isMediaPlaying: false

    Header {
        id: header
        height: 100

        // Forward Header signals to MainView signals
        onViewVolume: mainView.viewVolume()
        onViewBrightness: mainView.viewBrightness()
        onViewBluetoothStatus: mainView.viewBluetoothStatus()
        onViewWifiStatus: mainView.viewWifiStatus()
    }

    // Container for menus (Media/Main)
    Item {
        id: menuContainer
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        // Use states to control which menu is visible
        states: [
            State {
                name: "mainMenu"
                when: !mainView.isMediaPlaying
                PropertyChanges { target: mainMenu; opacity: 1 }
                PropertyChanges { target: mediaMenu; opacity: 0 }
            },
            State {
                name: "mediaMenu"
                when: mainView.isMediaPlaying
                PropertyChanges { target: mainMenu; opacity: 0 }
                PropertyChanges { target: mediaMenu; opacity: 1 }
            }
        ]

        // Define transitions for smooth change between states
        transitions: [
            Transition {
                from: "*"; to: "*"
                ParallelAnimation {
                    NumberAnimation { properties: "opacity"; duration: 500; easing.type: Easing.InOutQuad }
                }
            }
        ]

        MediaNotPlayingMenu {
            id: mainMenu
            width: 660
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 0 : 1 // Initial opacity based on isMediaPlaying
            Connections {
                target: mainMenu
                onViewSettings: mainView.viewSettings()

            }
        }

        MediaPlayingMenu {
            id: mediaMenu
            width: 800
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 1 : 0 // Initial opacity based on isMediaPlaying
            Connections {
                target: mediaMenu
                onViewSettings: mainView.viewSettings()

            }
        }
    }
}