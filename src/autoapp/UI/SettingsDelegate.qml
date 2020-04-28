import QtQuick 2.7
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

ItemDelegate {
    id: delegate

    checkable: false

    contentItem: ColumnLayout {
        spacing: 10

        Label {
            text: optionName
            font.bold: false
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        GridLayout {
            id: grid
            visible: false

            columns: 1
            rowSpacing: 10
            columnSpacing: 10
        }
    }
}