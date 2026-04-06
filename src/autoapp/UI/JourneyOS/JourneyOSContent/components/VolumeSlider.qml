import QtQuick
import QtQuick.Controls
import JourneyOS 1.0

Slider {
    id: volumeSlider
    from: volumePopupHandler.volumeSinkMin
    to: volumePopupHandler.volumeSinkMax
    stepSize: 1
    snapMode: Slider.SnapAlways
    value: volumePopupHandler.volumeSink
    orientation: Qt.Vertical
    background: baseBackground
    property color sliderColor: Constants.sliderPrimaryColor
    property color alternateColor: Constants.sliderAlternateColor

    Rectangle {
        id: baseBackground
        x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
        y: volumeSlider.topPadding
        implicitWidth: 20
        implicitHeight: 200
        width: implicitWidth
        height: volumeSlider.availableHeight
        radius: 10
        color: Constants.sliderBackgroundColor

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
        color: Constants.sliderHandle
        border.color: Constants.sliderHandleBorder
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
                color: Constants.sliderHandlePressed
                border.color: Constants.textPrimary
            }
        }
    ]
}
