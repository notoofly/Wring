import QtQuick

Item {
    id: ringItemRoot

    property var itemData: null
    property bool isSelected: false
    property var ringCenter: ({x: 0, y: 0})
    property real ringRadius: 150
    property int itemIndex: 0
    property int totalItems: 0

    signal clicked()

    scale: isSelected ? 1.15 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 150
            easing.type: Easing.OutQuad
        }
    }
}
