pragma Singleton
import QtQuick
import QtQuick.Studio.Application

QtObject {
    readonly property int width: 800
    readonly property int height: 480

    property string relativeFontDirectory: "fonts"

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Qt.application.font.family,
                                             pixelSize: Qt.application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Qt.application.font.family,
                                                  pixelSize: Qt.application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: "#EAEAEA"


    readonly property color primaryBackgroundColor: "#000000"           // Primary View
    readonly property color settingsBackgroundColor: "#a3caed"          // Settings Window
    readonly property color settingsPopupBackgroundColor: "#55a3caed"   // Settings Popups - eg Bluetooth, Wifi
    readonly property color sliderPopupBackgroundColor: "#557900b5"

    // Status/Interactive Buttons
    readonly property color okColor: "#009900"                          // OK - eg Connected
    readonly property color waitColor: "#999900"                        // Waiting - eg Indeterminate
    readonly property color badColor: "#990000"                         // Bad - eg Disconnected
    readonly property color actionColor: "#000099"                      // Actioning - eg Connecting/Ready
    readonly property color baseColor: "#999999"                        // Base Color

    readonly property color sliderPrimaryColor: "#cc2d004d"             // Slider Main Color
    readonly property color sliderAlternateColor: "#d113a8fd"           // Slider Alternate Slide Color
    readonly property color sliderBackgroundColor: "#77bdbebf"          // Slider Background Color

    readonly property color buttonBackgroundColor: "#55999999"          // Button / Up - Background
    readonly property color buttonPressedBackgroundColor: "#bb999999"   // Button / Down - Background
    readonly property color buttonColor: "#ffffff"                      // Button / Up - Text/Icon Color
    readonly property color buttonPressedColor: "#80ffffff"             // Button / Down - Text/Icon Color

    // 2. REPLACE StudioApplication WITH QtObject
    //property QtObject application: QtObject {
        // We keep the property name 'fontPath' so other files don't break.
        // We keep the logic valid (resolving the URL).
      //  property string fontPath: Qt.resolvedUrl("../JourneyOSContent/" + relativeFontDirectory)
    //}

    property StudioApplication application: StudioApplication {
        fontPath: Qt.resolvedUrl("../JourneyOSContent/" + relativeFontDirectory)
    }
}
