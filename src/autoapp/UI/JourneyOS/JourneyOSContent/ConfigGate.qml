pragma Singleton
import QtQuick

QtObject {
    // True when this app should show hardware-config UI
    // (head unit mode, OR running on Linux with direct device access)
    readonly property bool showConfig: settingsViewHandler.headUnitMode
                                    || (Qt.platform.os === "linux" && !isSystemManaged)
}
