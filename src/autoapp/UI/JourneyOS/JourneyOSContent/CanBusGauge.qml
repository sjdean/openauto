import QtQuick
import JourneyOS

// Arc gauge for continuously varying values (speed, RPM).
// Draws a 270° sweep from lower-left (7 o'clock) to lower-right (5 o'clock)
// going clockwise through 12 o'clock.
Item {
    id: gauge

    property real value:    0
    property real minValue: 0
    property real maxValue: 100
    property string label:  ""
    property string unit:   ""

    // Colour thresholds: low (safe) → mid (caution) → high (danger)
    property color colorLow:  "#4CAF50"   // green
    property color colorMid:  "#FF9800"   // orange
    property color colorHigh: "#F44336"   // red
    property real  midPoint:  0.75        // fraction at which mid transitions to high

    function gaugeColor(fraction) {
        fraction = Math.max(0.0, Math.min(1.0, fraction))
        var c1, c2, t
        if (fraction < midPoint) {
            c1 = colorLow; c2 = colorMid; t = fraction / midPoint
        } else {
            c1 = colorMid; c2 = colorHigh; t = (fraction - midPoint) / (1.0 - midPoint)
        }
        return Qt.rgba(c1.r + (c2.r - c1.r) * t,
                       c1.g + (c2.g - c1.g) * t,
                       c1.b + (c2.b - c1.b) * t, 1.0)
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            var cx = width  / 2
            var cy = height / 2
            var r  = Math.min(width, height) * 0.38
            var lw = Math.max(4, r * 0.18)

            // Arc spans 270°: start at 135° (canvas = clockwise from east)
            var startRad = 135 * Math.PI / 180
            var endRad   = 405 * Math.PI / 180   // 135 + 270

            var fraction  = (gauge.value - gauge.minValue) / Math.max(1, gauge.maxValue - gauge.minValue)
            fraction = Math.max(0.0, Math.min(1.0, fraction))
            var valueRad  = startRad + fraction * (endRad - startRad)

            // Background track
            ctx.strokeStyle = "#2a2a2a"
            ctx.lineWidth   = lw
            ctx.lineCap     = "round"
            ctx.beginPath()
            ctx.arc(cx, cy, r, startRad, endRad, false)
            ctx.stroke()

            // Value arc
            if (fraction > 0.001) {
                var col = gauge.gaugeColor(fraction)
                ctx.strokeStyle = col
                ctx.lineWidth   = lw
                ctx.lineCap     = "round"
                ctx.beginPath()
                ctx.arc(cx, cy, r, startRad, valueRad, false)
                ctx.stroke()
            }
        }

        onWidthChanged:  requestPaint()
        onHeightChanged: requestPaint()
    }

    // Value text
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter:   parent.verticalCenter
        anchors.verticalCenterOffset: -parent.height * 0.08
        text:       Math.round(gauge.value)
        font.pixelSize: Math.max(12, parent.width * 0.22)
        font.bold:  true
        color:      "white"
    }

    // Unit text
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter:   parent.verticalCenter
        anchors.verticalCenterOffset: parent.height * 0.12
        text:           gauge.unit
        font.pixelSize: Math.max(8, parent.width * 0.10)
        color:          "#aaaaaa"
    }

    // Label at bottom
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom:           parent.bottom
        anchors.bottomMargin:     2
        text:           gauge.label
        font.pixelSize: Math.max(8, parent.width * 0.09)
        color:          "#888888"
    }

    onValueChanged:    canvas.requestPaint()
    onMinValueChanged: canvas.requestPaint()
    onMaxValueChanged: canvas.requestPaint()
}