/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls

Slider {
    id: volumeSlider
    value: 0.5
    orientation: Qt.Vertical
    background: baseBackground
    property color sliderColor: "#cc2d004d"
    property color alternateColor: "#cc1334fd"

    Rectangle {
        id: baseBackground
        x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
        y: volumeSlider.topPadding
        implicitWidth: 20
        implicitHeight: 200
        parent: volumeSlider.background
        width: implicitWidth
        height: volumeSlider.availableHeight
        radius: 10
        color: "#77bdbebf"

        Rectangle {
            y: ((volumeSlider.availableHeight - handleItem.height) * volumeSlider.visualPosition) + volumeSlider.topPadding
            id: groove
            height: volumeSlider.availableHeight - ((volumeSlider.availableHeight - handleItem.height) * volumeSlider.visualPosition) - volumeSlider.topPadding
            width: parent.width
            radius: 10

            Behavior on color {
                ColorAnimation { duration: 100 }
            }

            // Calculate color based on slider position
            color: Qt.hsla(
                       // Hue interpolation
                       (Qt.hsla(sliderColor.hslHue, 0, 0, 1).hslHue * (1 - volumeSlider.visualPosition)) +
                       (Qt.hsla(alternateColor.hslHue, 0, 0, 1).hslHue * volumeSlider.visualPosition),
                       // Saturation interpolation
                       (sliderColor.hslSaturation * (1 - volumeSlider.visualPosition)) + (alternateColor.hslSaturation * volumeSlider.visualPosition),
                       // Lightness interpolation
                       (sliderColor.hslLightness * (1 - volumeSlider.visualPosition)) + (alternateColor.hslLightness * volumeSlider.visualPosition),
                       // Alpha interpolation
                       (sliderColor.a * (1 - volumeSlider.visualPosition)) + (alternateColor.a * volumeSlider.visualPosition)
                   )
        }
    }

    handle: Item {
        x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
        y: ((volumeSlider.availableHeight - handleItem.height) * volumeSlider.visualPosition) + volumeSlider.topPadding
        height: handleItem.height
        width: handleItem.width
    }

    Rectangle {
        id: handleItem
        parent: volumeSlider.handle
        implicitWidth: 26
        implicitHeight: 26
        radius: 13
        color: "#f6f6f6"
        border.color: "#bdbebf"
    }


    states: [
        State {
            name: "normal"
            when: !volumeSlider.pressed
        },
        State {
            name: "pressed"
            when: volumeSlider.pressed

            PropertyChanges {
                target: handleItem
                color: "#cc2d004d"
                border.color: "#ffffff"
            }
        }
    ]
}
