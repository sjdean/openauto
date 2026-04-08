import QtQuick
import QtQuick.Controls
import JourneyOS 1.0

Slider {
    id: brightnessSlider
    from: brightnessPopupHandler.currentMin
    to: brightnessPopupHandler.currentMax
    stepSize: 1
    snapMode: Slider.SnapAlways
    orientation: Qt.Vertical
    background: baseBackground
    value: brightnessPopupHandler.targetBrightness

    // Single-hue intensity variation: dark purple (dim) → softer purple (bright)
    property color sliderColor:    Constants.sliderBrightnessTrackLow
    property color alternateColor: Constants.sliderBrightnessTrackHigh

    Rectangle {
        id: baseBackground
        x: brightnessSlider.leftPadding + brightnessSlider.availableWidth / 2 - width / 2
        y: brightnessSlider.topPadding
        implicitWidth: Constants.sliderTrackWidth
        implicitHeight: Constants.sliderTrackHeight
        width: implicitWidth
        height: brightnessSlider.availableHeight
        radius: Constants.shapeSlider
        color: Constants.sliderInactiveTrack

        Rectangle {
            y: ((brightnessSlider.availableHeight - handleItem.height) * brightnessSlider.visualPosition) + brightnessSlider.topPadding
            id: groove
            height: brightnessSlider.availableHeight - ((brightnessSlider.availableHeight - handleItem.height) * brightnessSlider.visualPosition) - brightnessSlider.topPadding
            width: parent.width
            radius: Constants.shapeSlider

            Behavior on color {
                ColorAnimation { duration: 100 }
            }

            color: Qt.hsla(
                       (Qt.hsla(sliderColor.hslHue, 0, 0, 1).hslHue * (1 - brightnessSlider.visualPosition)) +
                       (Qt.hsla(alternateColor.hslHue, 0, 0, 1).hslHue * brightnessSlider.visualPosition),
                       (sliderColor.hslSaturation * (1 - brightnessSlider.visualPosition)) + (alternateColor.hslSaturation * brightnessSlider.visualPosition),
                       (sliderColor.hslLightness * (1 - brightnessSlider.visualPosition)) + (alternateColor.hslLightness * brightnessSlider.visualPosition),
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
        implicitWidth: Constants.sliderHandleSize
        implicitHeight: Constants.sliderHandleSize
        radius: Constants.sliderHandleSize / 2
        color: Constants.sliderHandle
        border.color: Constants.sliderHandleOutline
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
                color: Constants.sliderHandlePressed
                border.color: Constants.onSurface
            }
        }
    ]
}
