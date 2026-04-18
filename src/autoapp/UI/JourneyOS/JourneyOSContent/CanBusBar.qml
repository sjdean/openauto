import QtQuick
import JourneyOS

// Vertical fill bar — for fuel range and battery level.
// Fills from bottom; colour lerps from colorEmpty (red) to colorFull (green).
Item {
    id: bar

    property real   value:      0
    property real   minValue:   0
    property real   maxValue:   100
    property string label:      ""
    property string valueText:  Math.round(bar.value).toString()
    property color  colorEmpty: "#F44336"   // red
    property color  colorFull:  "#4CAF50"   // green

    function barColor(fraction) {
        fraction = Math.max(0.0, Math.min(1.0, fraction))
        var c1 = colorEmpty, c2 = colorFull
        return Qt.rgba(c1.r + (c2.r - c1.r) * fraction,
                       c1.g + (c2.g - c1.g) * fraction,
                       c1.b + (c2.b - c1.b) * fraction, 1.0)
    }

    property real fraction: Math.max(0.0, Math.min(1.0,
        (value - minValue) / Math.max(1, maxValue - minValue)))

    // Pill-shaped background track
    Rectangle {
        id: track
        width:  parent.width * 0.35
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top:    parent.top
        anchors.bottom: valueLabel.top
        anchors.bottomMargin: 4
        color:  "#2a2a2a"
        radius: width / 2

        // Fill
        Rectangle {
            width:  parent.width
            height: parent.height * bar.fraction
            anchors.bottom: parent.bottom
            color:  bar.barColor(bar.fraction)
            radius: parent.radius
        }
    }

    // Numeric value below bar
    Text {
        id: valueLabel
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: labelText.top
        anchors.bottomMargin: 1
        text:           bar.valueText
        font.pixelSize: Math.max(8, bar.width * 0.18)
        font.bold:      true
        color:          "white"
    }

    Text {
        id: labelText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        text:           bar.label
        font.pixelSize: Math.max(7, bar.width * 0.14)
        color:          "#888888"
    }
}