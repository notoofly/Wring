import QtQuick

Item {
    id: settingsItemRoot

    property string title: ""
    property bool isSelected: false

    width: 64
    height: 64

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#443344"
        border.color: isSelected ? "#ff8800" : "#665566"
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
        text: "\u2699"
        color: isSelected ? "#ff8800" : "#aaaaaa"
        font.pixelSize: 28

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
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
