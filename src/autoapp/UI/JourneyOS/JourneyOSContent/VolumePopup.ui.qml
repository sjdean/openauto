import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import JourneyOS

Item {
    id: volumePopup
    width: 75
    height: 380

    property bool isMuted: false

    signal mutePressed(bool isMuted)


    Column {
        id: volumeColumn
        width: 50
        height: volumePopup.height - 24
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter


        VolumeSlider {
            id: volumeSlider
            height: volumePopup.height - 89
            anchors.top: parent.top
            anchors.topMargin: 0
            alternateColor: Constants.sliderAlternateColor
            anchors.horizontalCenter: parent.horizontalCenter
            onValueChanged: volumePopupHandler.volumeSink = value

        }

        JourneyButton {
            id: muteButton
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            iconSize: 24
            icon.source: "images/volume-mute.svg"
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.width
            text: "Mute"
            width: parent.width
            Connections {
                target: muteButton
                onClicked: {
                    volumePopup.isMuted = !volumePopup.isMuted
                    volumePopupHandler.volumeSinkMute = volumePopup.isMuted
                }

            }
        }
    }

    states: [
        State {
            name: "muted"
            when: volumePopup.isMuted
            PropertyChanges {
                target: muteButton
                icon.color: "#ff0000"
            }
            PropertyChanges {
                target: volumeSlider
                sliderColor: Constants.sliderPrimaryColor
            }
        },
        State {
            name: "notmuted"
            when: !volumePopup.isMuted
            PropertyChanges {
                target: muteButton
                icon.color: "#ffffff"
            }
            PropertyChanges {
                target: volumeSlider
            }
        }
    ]
}
