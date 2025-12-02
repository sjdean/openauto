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
        spacing: 5

        BrightnessSlider {
            id: brightnessSlider
            height: parent.height
            width: parent.width
            alternateColor: "#ab381353"
            onValueChanged: brightnessPopupHandler.brightness = value
        }
    }
}
