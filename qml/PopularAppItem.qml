import QtQuick

Item {
    id: popularAppItemRoot

    property string title: ""
    property var icon: null
    property bool isSelected: false

    width: 64
    height: 64

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#444466"
        border.color: isSelected ? "#ffffff" : "#555555"
        border.width: isSelected ? 3 : 1

        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }

        Behavior on color {
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
            return "A"
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
