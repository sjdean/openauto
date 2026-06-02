import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import JourneyOS

Item {
    id: volumePopup
    anchors.fill: parent

    property bool isMuted: volumePopupHandler.volumeSinkMute


    Column {
        id: volumeColumn
        width: parent.width * 0.66
        height: volumePopup.height - 24
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 5


        VolumeSlider {
            id: volumeSlider
            height: parent.height - muteButton.height - parent.spacing
            width: parent.width
            alternateColor: Constants.sliderAlternateColor
            value: volumePopupHandler.volumeSink
            onMoved: volumePopupHandler.volumeSink = value
            onPressedChanged: if (!pressed) volumePopupHandler.saveSettings()
        }

        JourneyButton {
            id: muteButton
            iconSize: height * 0.5
            icon.source: "images/volume.svg"
            height: parent.width
            text: "Mute"
            width: parent.width
            Connections {
                target: muteButton
                function onClicked() { volumePopupHandler.toggleSinkMute() }
            }
        }
    }

    states: [
        State {
            name: "muted"
            when: volumePopup.isMuted
            PropertyChanges {
                target: muteButton
                icon.source: "images/volume-mute.svg"
                textColor: "#ff0000"
                text: "Unmute"
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
                icon.source: "images/volume.svg"
                textColor: Constants.btnContent
                text: "Mute"
            }
        }
    ]
}
