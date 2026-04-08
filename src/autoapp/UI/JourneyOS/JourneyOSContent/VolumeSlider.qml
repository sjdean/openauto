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
    property color sliderColor: Constants.sliderActiveTrackLow
    property color alternateColor: Constants.sliderActiveTrackHigh

    Rectangle {
        id: baseBackground
        x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
        y: volumeSlider.topPadding
        implicitWidth: Constants.sliderTrackWidth
        implicitHeight: Constants.sliderTrackHeight
        width: implicitWidth
        height: volumeSlider.availableHeight
        radius: Constants.shapeSlider
        color: Constants.sliderInactiveTrack

        Rectangle {
            y: ((volumeSlider.availableHeight - handleItem.height) * volumeSlider.visualPosition) + volumeSlider.topPadding
            id: groove
            height: volumeSlider.availableHeight - ((volumeSlider.availableHeight - handleItem.height) * volumeSlider.visualPosition) - volumeSlider.topPadding
            width: parent.width
            radius: Constants.shapeSlider

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
        implicitWidth: Constants.sliderHandleSize
        implicitHeight: Constants.sliderHandleSize
        radius: Constants.sliderHandleSize / 2
        color: Constants.sliderHandle
        border.color: Constants.sliderHandleOutline
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
