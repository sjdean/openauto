import QtQuick

MouseArea {
    id: clickOutsideHandler
    anchors.fill: parent
    propagateComposedEvents: true

    property Item targetItem // Reference to the item to check if clicked inside

    onPressed: {
        if (!targetItem.contains(mouse)) {
            targetItem.visible = false;
        }
        mouse.accepted = false; // Allow event propagation
    }
}
