import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import JourneyOS

Item {
    id: brightnessPopup
    width: 75
    height: 380

    Column {
        id: brightnessColumn
        width: 50
        height: brightnessPopup.height - 24
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter


        BrightnessSlider {
            id: brightnessSlider
            height: brightnessPopup.height - 24
            anchors.top: parent.top
            anchors.topMargin: 0
            alternateColor: "#ab381353"
            anchors.horizontalCenter: parent.horizontalCenter
            onValueChanged: brightnessPopupHandler.brightness = value
        }
    }

}
