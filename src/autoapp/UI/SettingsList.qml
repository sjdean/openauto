import QtQuick 2.7
import QtQuick.Controls 2.1

ListView {
    id: listView

    signal press(int index)

    width: 320
    height: 480

    focus: true
    boundsBehavior: Flickable.StopAtBounds

    delegate: SettingsDelegate {
        id: delegate
        width: listView.width

        Connections {

            target: delegate
            onPressed: listView.press(index)
        }
    }

    model: SettingModel {
        id: settingsModel
    }

    ScrollBar.vertical: ScrollBar { }
}
