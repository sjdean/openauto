import QtQuick
import QtQuick.Controls
import JourneyOS

Item {
    id: mainView
    width: parent ? parent.width : 800
    height: parent ? parent.height : 480
    objectName: "MainView" // Useful for debugging

    // Signals to tell Journey.qml what to do

    // Menu Buttons
    signal viewSettings()
    signal viewAndroidAuto()
    signal viewPower()
    signal viewOBD()
    signal viewNavigation()
    signal viewUSB()
    signal viewRadio()
    signal viewBluetooth()

    // Slider Controls
    signal viewVolume()

    signal viewBrightness()

    // Status Buttons
    signal viewBluetoothStatus()

    signal viewWifiStatus()

    signal viewAndroidAutoStatus()

    property bool isMediaPlaying: false

    Header {
        id: header
        height: 100

        // Forward Header signals to MainView signals
        onViewVolume: mainView.viewVolume()
        onViewBrightness: mainView.viewBrightness()
        onViewBluetoothStatus: mainView.viewBluetoothStatus()
        onViewWifiStatus: mainView.viewWifiStatus()
        onViewAndroidAutoStatus: mainView.viewAndroidAutoStatus()

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
                PropertyChanges {
                    target: mainMenu; opacity: 1
                }
                PropertyChanges {
                    target: mediaMenu; opacity: 0
                }
            },
            State {
                name: "mediaMenu"
                when: mainView.isMediaPlaying
                PropertyChanges {
                    target: mainMenu; opacity: 0
                }
                PropertyChanges {
                    target: mediaMenu; opacity: 1
                }
            }
        ]

        // Define transitions for smooth change between states
        transitions: [
            Transition {
                from: "*";
                to: "*"
                ParallelAnimation {
                    NumberAnimation {
                        properties: "opacity"; duration: 500; easing.type: Easing.InOutQuad
                    }
                }
            }
        ]

        MediaNotPlayingMenu {
            id: mainMenu
            width: parent.width
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 0 : 1 // Initial opacity based on isMediaPlaying
            Connections {
                target: mainMenu
                onViewSettings: mainView.viewSettings()
                onViewAndroidAuto: mainView.viewAndroidAuto()
                onViewPower: mainMenu.viewPower()
                onViewOBD: mainMenu.viewOBD()
                onViewNavigation: mainMenu.viewNavigation()
                onViewRadio: mainMenu.viewRadio()
                onViewUSB: mainMenu.viewUSB()
                onViewBluetooth: mainMenu.viewBluetooth()
            }
        }

        MediaPlayingMenu {
            id: mediaMenu
            width: parent.width
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true
            opacity: mainView.isMediaPlaying ? 1 : 0 // Initial opacity based on isMediaPlaying
            Connections {
                target: mediaMenu
                onViewSettings: mainView.viewSettings()
                onViewAndroidAuto: mainView.viewAndroidAuto()
                onViewPower: mainMenu.viewPower()
                onViewOBD: mainMenu.viewOBD()
                onViewNavigation: mainMenu.viewNavigation()
                onViewRadio: mainMenu.viewRadio()
                onViewUSB: mainMenu.viewUSB()
                onViewBluetooth: mainMenu.viewBluetooth()
            }
        }
    }
}