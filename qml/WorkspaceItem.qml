import QtQuick

Item {
    id: workspaceItemRoot

    property string title: ""
    property bool isActive: false
    property bool isSelected: false

    width: 64
    height: 64

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: isActive ? "#22aa44" : "#333333"
        border.color: isSelected ? "#ffffff" : "#555555"
        border.width: isSelected ? 3 : 1

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }

        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }
    }

    Text {
        anchors.centerIn: parent
        text: {
            if (title.length > 0) {
                return title.charAt(0).toUpperCase()
            }
            return "W"
        }
        color: "#ffffff"
        font.pixelSize: 20
        font.bold: true
    }

    Text {
        anchors.top: parent.bottom
        anchors.topMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        text: title
        color: "#cccccc"
        font.pixelSize: 10
        width: 80
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
}
