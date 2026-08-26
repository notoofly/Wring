import QtQuick

Item {
    id: ring2ButtonRoot

    property bool isSelected: false

    width: 64
    height: 64

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#444444"
        border.color: isSelected ? "#ff8800" : "#666666"
        border.width: isSelected ? 3 : 2

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
        text: "+"
        color: isSelected ? "#ff8800" : "#aaaaaa"
        font.pixelSize: 28
        font.bold: true

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
        text: "More"
        color: "#cccccc"
        font.pixelSize: 10
    }
}
