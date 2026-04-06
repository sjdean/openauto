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

    // Show the home button and restart the idle hide timer
    function revealHomeBtn() {
        if (settingsViewHandler.androidAutoHomeButtonVisibility !== "alwaysVisible") {
            homeBtn.opacity = 1
            homeBtnHideTimer.restart()
        }
    }

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
        onPressed:  (mouse) => { inputMapper.handleMouseEvent(0, mouse.x, mouse.y); revealHomeBtn() }
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
            revealHomeBtn()
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
    // A subtle floating home button — fades in on touch, auto-hides after idle
    RoundButton {
        id: homeBtn
        text: "⌂"
        width: Math.max(72, parent.width * 0.09)
        height: width
        font.pointSize: 24

        // Positioning
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: Math.max(30, parent.width * 0.0375)
        z: 999

        // Visibility: alwaysVisible shows at full opacity, touchToReveal starts hidden
        opacity: settingsViewHandler.androidAutoHomeButtonVisibility === "alwaysVisible" ? 1 : 0

        Behavior on opacity { NumberAnimation { duration: 200 } }

        // Content (Icon)
        contentItem: Text {
            text: homeBtn.text
            font: homeBtn.font
            color: Constants.textPrimary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: homeBtn.down ? 0.7 : 1.0
        }

        // Background
        background: Rectangle {
            radius: homeBtn.width / 2
            color: Constants.aaHomeBtnBg
            border.color: Constants.aaHomeBtnBorder
            border.width: 2
            opacity: homeBtn.down ? 0.9 : 0.6
        }

        onClicked: root.requestHome()
    }

    // Auto-hide timer: hides the home button after 3 seconds of idle
    Timer {
        id: homeBtnHideTimer
        interval: 3000
        running: false
        onTriggered: homeBtn.opacity = 0
    }

    // Cleanup
    Component.onDestruction: videoBackend.setVideoSink(null)
}