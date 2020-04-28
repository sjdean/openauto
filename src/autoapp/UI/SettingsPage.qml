import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

ScrollablePage {
    id: settingsPage

    property int currentSetting: -1

    SettingsList {
        id: settingsList
        anchors.fill: parent
        onPress: {
            currentSetting = index
            // navigate to setting dialog / page
        }
    }
}
