import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import JourneyOS

Item {
    id: brightnessPopup
    anchors.fill: parent

    Column {
        id: brightnessColumn
        width: parent.width * 0.66
        height: brightnessPopup.height - 24
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 5

        BrightnessSlider {
            id: brightnessSlider
            height: parent.height - darkModeButton.height - parent.spacing
            width: parent.width
            alternateColor: Constants.sliderBrightnessTrackHigh
            onMoved: brightnessPopupHandler.targetBrightness = value
        }

        JourneyButton {
            id: darkModeButton
            iconSize: height * 0.5
            icon.source: settingsViewHandler.uiDarkMode ? "images/night.svg" : "images/day.svg"
            height: parent.width
            text: settingsViewHandler.uiDarkMode ? "Dark" : "Light"
            width: parent.width
            Connections {
                target: darkModeButton
                onClicked: settingsViewHandler.uiDarkMode = !settingsViewHandler.uiDarkMode
            }
        }
    }

    states: [
        State {
            name: "dark"
            when: settingsViewHandler.uiDarkMode
            PropertyChanges {
                target: darkModeButton
                icon.color: Constants.palettePrimary
            }
        },
        State {
            name: "light"
            when: !settingsViewHandler.uiDarkMode
            PropertyChanges {
                target: darkModeButton
                icon.color: Constants.textSecondary
            }
        }
    ]
}
