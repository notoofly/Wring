import QtQuick
import QtQuick.Window

Window {
    id: root
    visible: true
    width: 1
    height: 1
    x: -100
    y: -100
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    title: "Wring"
    visible: false

    visibility: Window.Hidden

    Wring {
        id: wring
        anchors.fill: parent
    }

    Connections {
        target: WringController

        function onVisibleChanged() {
            if (WringController.visible) {
                var geom = WringController.screenGeometry
                root.width = geom.width
                root.height = geom.height
                root.x = geom.x
                root.y = geom.y
                root.visibility = Window.FullScreen
                root.show()
                root.raise()
                root.requestActivate()
            } else {
                root.visibility = Window.Hidden
                root.hide()
            }
        }
    }

    Component.onCompleted: {
        root.visibility = Window.Hidden
    }
}
