import QtQuick
import QtQuick.Controls
import QtMultimedia
import JourneyOS 1.0

Item {
    id: root
    objectName: "AndroidAutoView" // CRITICAL: Matches check in Journey.qml

    signal requestHome()
    signal viewVolume()
    signal viewBrightness()
    signal viewSettings()

    // 1. VIDEO LAYER
    VideoOutput {
        id: aaVideoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        Component.onCompleted: videoBackend.setVideoSink(aaVideoOutput.videoSink)
    }

    // 2. MOUSE HANDLER (Works on Mac/PC/Simulated Mouse)
    MouseArea {
        anchors.fill: parent
        z: 5 // Lower Z-index than the button (which is usually auto-z or defined last)

        // This area will catch actual mouse clicks.
        // On a pure touchscreen driver (without mouse emulation), this does nothing.
        onPressed:  (mouse) => inputMapper.handleMouseEvent(0, mouse.x, mouse.y)
        onReleased: (mouse) => inputMapper.handleMouseEvent(1, mouse.x, mouse.y)
        onPositionChanged: (mouse) => inputMapper.handleMouseEvent(2, mouse.x, mouse.y)
    }

    // 3. TOUCH HANDLER (Works on the actual Screen)
    MultiPointTouchArea {
        anchors.fill: parent
        z: 5

        // CRITICAL: Prevent this area from stealing Mouse events!
        mouseEnabled: false

        onPressed: (points) => {
            // We only care about the first finger for standard AA usage
            if (points.length > 0)
                inputMapper.handleTouchEvent(0, points[0].x, points[0].y)
        }
        onReleased: (points) => {
            if (points.length > 0)
                inputMapper.handleTouchEvent(1, points[0].x, points[0].y)
        }
        onUpdated: (points) => {
            if (points.length > 0)
                inputMapper.handleTouchEvent(2, points[0].x, points[0].y)
        }
    }

    // 4. OVERLAY LAYER (UI)
    // A subtle floating home button
    RoundButton {
        id: homeBtn
        text: "⌂"
        width: 72  // Slightly larger for easy car touch
        height: 72
        font.pixelSize: 36

        // Positioning
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 30
        z: 999

        // Content (Icon)
        contentItem: Text {
            text: homeBtn.text
            font: homeBtn.font
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: homeBtn.down ? 0.7 : 1.0 // Visual feedback on press
        }

        // Background
        background: Rectangle {
            radius: homeBtn.width / 2
            color: "#80000000" // 50% Black
            border.color: "#40FFFFFF" // Subtle white border
            border.width: 2

            // Visual Feedback: Darker when pressed
            opacity: homeBtn.down ? 0.9 : 0.6
        }

        onClicked: root.requestHome()
    }

    // Cleanup
    Component.onDestruction: videoBackend.setVideoSink(null)
}