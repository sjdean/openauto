import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import JourneyOS

Item {
    id: brightnessPopup
    anchors.fill: parent

    property bool isDarkMode: false

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
            height: parent.height
            width: parent.width
            alternateColor: Constants.sliderBrightnessHigh
            onValueChanged: brightnessPopupHandler.targetBrightness = value
            value: brightnessPopupHandler.targetBrightness
        }
        // TODO: Temporarily remove Dark mode Button
/*
        JourneyButton {
            id: darkModeButton

            iconSize: height * 0.5
            icon.source: "images/volume-mute.svg"
            height: parent.width
            text: "Dark Mode"
            width: parent.width
            Connections {
                target: darkModeButton
                onClicked: {
                    brightnessPopup.isDarkMode = !brightnessPopup.isDarkMode
                }
            }
        }*/
    }
/*
    states: [
        State {
            name: "dark"
            when: brightnessPopup.isDarkMode
            PropertyChanges {
                target: darkModeButton
                icon.color: "#ff0000"
            }
            PropertyChanges {
                target: brightnessSlider
                sliderColor: Constants.sliderPrimaryColor
            }
        },
        State {
            name: "light"
            when: !brightnessPopup.isDarkMode
            PropertyChanges {
                target: darkModeButton
                icon.color: "#ffffff"
            }
            PropertyChanges {
                target: brightnessSlider
            }
        }
    ]*/
}
