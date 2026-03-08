/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls

Slider {
    id: brightnessSlider
    from: brightnessPopupHandler.currentMin
    to: brightnessPopupHandler.currentMax
    stepSize: 1
    snapMode: Slider.SnapAlways
    orientation: Qt.Vertical
    background: baseBackground
    value: brightnessPopupHandler.targetBrightness
    property color sliderColor: "#cc2d004d"
    property color alternateColor: "#ab381353"

    Rectangle {
        id: baseBackground
        x: brightnessSlider.leftPadding + brightnessSlider.availableWidth / 2 - width / 2
        y: brightnessSlider.topPadding
        implicitWidth: 20
        implicitHeight: 200
        width: implicitWidth
        height: brightnessSlider.availableHeight
        radius: 10
        color: "#77bdbebf"

        Rectangle {
            y: ((brightnessSlider.availableHeight - handleItem.height) * brightnessSlider.visualPosition) + brightnessSlider.topPadding
            id: groove
            height: brightnessSlider.availableHeight - ((brightnessSlider.availableHeight - handleItem.height) * brightnessSlider.visualPosition) - brightnessSlider.topPadding
            width: parent.width
            radius: 10

            Behavior on color {
                ColorAnimation { duration: 100 }
            }

            // Calculate color based on slider position
            color: Qt.hsla(
                       // Hue interpolation
                       (Qt.hsla(sliderColor.hslHue, 0, 0, 1).hslHue * (1 - brightnessSlider.visualPosition)) +
                       (Qt.hsla(alternateColor.hslHue, 0, 0, 1).hslHue * brightnessSlider.visualPosition),
                       // Saturation interpolation
                       (sliderColor.hslSaturation * (1 - brightnessSlider.visualPosition)) + (alternateColor.hslSaturation * brightnessSlider.visualPosition),
                       // Lightness interpolation
                       (sliderColor.hslLightness * (1 - brightnessSlider.visualPosition)) + (alternateColor.hslLightness * brightnessSlider.visualPosition),
                       // Alpha interpolation
                       (sliderColor.a * (1 - brightnessSlider.visualPosition)) + (alternateColor.a * brightnessSlider.visualPosition)
                   )
        }
    }

    handle: Item {
        x: brightnessSlider.leftPadding + brightnessSlider.availableWidth / 2 - width / 2
        y: ((brightnessSlider.availableHeight - handleItem.height) * brightnessSlider.visualPosition) + brightnessSlider.topPadding
        height: handleItem.height
        width: handleItem.width
    }

    Rectangle {
        id: handleItem
        parent: brightnessSlider.handle
        implicitWidth: 26
        implicitHeight: 26
        radius: 13
        color: "#f6f6f6"
        border.color: "#bdbebf"
    }


    states: [
        State {
            name: "normal"
            when: !brightnessSlider.pressed
        },
        State {
            name: "pressed"
            when: brightnessSlider.pressed

            PropertyChanges {
                target: handleItem
                color: "#cc2d004d"
                border.color: "#ffffff"
            }
        }
    ]
}
