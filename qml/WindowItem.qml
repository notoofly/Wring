import QtQuick

Item {
    id: windowItemRoot

    property string title: ""
    property var icon: null
    property bool isActive: false
    property bool isSelected: false

    width: 64
    height: 64

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: isActive ? "#4488ff" : "#333333"
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

    Image {
        id: iconImage
        anchors.centerIn: parent
        width: 36
        height: 36
        source: icon ? "" : ""
        fillMode: Image.PreserveAspectFit
        visible: false
    }

    Text {
        anchors.centerIn: parent
        text: {
            if (title.length > 0) {
                return title.charAt(0).toUpperCase()
            }
            return "?"
        }
        color: "#ffffff"
        font.pixelSize: 20
        font.bold: true
        visible: !icon || iconImage.status !== Image.Ready
    }

    Text {
        anchors.top: parent.bottom
        anchors.topMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        text: title.length > 12 ? title.substring(0, 12) + "..." : title
        color: "#cccccc"
        font.pixelSize: 10
        width: 80
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
}
