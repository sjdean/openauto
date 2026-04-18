import QtQuick
import JourneyOS

// Horizontal L/R steering indicator.
// The L label intensifies towards activeColor as the wheel turns left;
// R label intensifies as it turns right.  A needle slides along the track.
//
// rawValue: the integer from canBusReceiver.steeringPosition
//           (decoded as (byte0*256+byte1)/100, unsigned, center ~327 for Laguna X56)
// center:   the raw value when wheels are straight (calibrate on the car)
// range:    how many raw units = full deflection either side
Item {
    id: steeringBar

    property int    rawValue:    0
    property int    center:      327       // calibrate to actual straight-ahead reading
    property int    range:       300       // raw units for full lock either side

    // Normalised position: -1.0 = full left, 0 = centre, +1.0 = full right
    property real   position: Math.max(-1.0, Math.min(1.0,
        (rawValue - center) / Math.max(1, range)))

    readonly property color neutralColor: "#444444"
    readonly property color activeColor:  "#2196F3"   // blue

    function lerpColor(c1, c2, t) {
        t = Math.max(0.0, Math.min(1.0, t))
        return Qt.rgba(c1.r + (c2.r - c1.r) * t,
                       c1.g + (c2.g - c1.g) * t,
                       c1.b + (c2.b - c1.b) * t, 1.0)
    }

    // L label
    Text {
        id: leftLabel
        anchors.left:           parent.left
        anchors.verticalCenter: track.verticalCenter
        text:      "L"
        font.pixelSize: 14
        font.bold: true
        color: steeringBar.position < 0
               ? steeringBar.lerpColor(steeringBar.neutralColor, steeringBar.activeColor,
                                       -steeringBar.position)
               : steeringBar.neutralColor
    }

    // Track
    Rectangle {
        id: track
        anchors.left:           leftLabel.right
        anchors.right:          rightLabel.left
        anchors.leftMargin:     6
        anchors.rightMargin:    6
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -8
        height: 5
        color:  "#2a2a2a"
        radius: 3

        // Centre mark
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter:   parent.verticalCenter
            width:  2
            height: 14
            color:  "#555555"
        }

        // Needle
        Rectangle {
            id: needle
            width:  6
            height: 22
            radius: 3
            anchors.verticalCenter: parent.verticalCenter
            x: (track.width / 2) + (steeringBar.position * (track.width / 2)) - width / 2
            color: Math.abs(steeringBar.position) < 0.02
                   ? "#666666"
                   : steeringBar.lerpColor(steeringBar.neutralColor, steeringBar.activeColor,
                                           Math.abs(steeringBar.position))

            Behavior on x { SmoothedAnimation { velocity: 400 } }
        }
    }

    // R label
    Text {
        id: rightLabel
        anchors.right:          parent.right
        anchors.verticalCenter: track.verticalCenter
        text:      "R"
        font.pixelSize: 14
        font.bold: true
        color: steeringBar.position > 0
               ? steeringBar.lerpColor(steeringBar.neutralColor, steeringBar.activeColor,
                                       steeringBar.position)
               : steeringBar.neutralColor
    }

    // Label
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom:           parent.bottom
        text:           "Steering"
        font.pixelSize: 10
        color:          "#666666"
    }
}