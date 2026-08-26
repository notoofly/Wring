import QtQuick
import QtQuick.Window

Item {
    id: wringRoot

    property real ring1Radius: 150
    property real ring2Radius: 120
    property real itemSize: 64
    property int animationDuration: 200

    visible: WringController.visible
    opacity: WringController.visible ? 1.0 : 0.0
    scale: WringController.visible ? 1.0 : 0.8

    Behavior on opacity {
        NumberAnimation {
            duration: wringRoot.animationDuration
            easing.type: Easing.OutQuad
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: wringRoot.animationDuration
            easing.type: Easing.OutBack
        }
    }

    Connections {
        target: WringController

        function onDismiss() {
            wringRoot.opacity = 0.0
            wringRoot.scale = 0.8
        }
    }

    Ring {
        id: ring1
        anchors.fill: parent
        visible: WringController.state === 1

        center: WringController.ring1Center
        radius: wringRoot.ring1Radius
        itemCount: WringController.ring1ItemCount
        selectedIndex: WringController.selectedIndex

        model: {
            var items = []
            var windows = WringController.windows

            for (var i = 0; i < windows.length; i++) {
                var win = windows[i]
                items.push({
                    type: "window",
                    title: win.title || "Untitled",
                    applicationName: win.applicationName || "",
                    icon: win.icon || null,
                    isActive: win.isActive || false,
                    id: win.id || 0
                })
            }

            items.push({
                type: "ring2button",
                title: "More",
                applicationName: "",
                icon: null,
                isActive: false,
                id: -1
            })

            return items
        }

        delegate: RingItem {
            width: wringRoot.itemSize
            height: wringRoot.itemSize

            itemData: modelData
            isSelected: index === WringController.selectedIndex
            ringCenter: WringController.ring1Center
            ringRadius: wringRoot.ring1Radius
            itemIndex: index
            totalItems: WringController.ring1ItemCount

            onClicked: {
                if (modelData.type === "ring2button") {
                    WringController.openRing2()
                } else {
                    WringController.selectByIndex(index)
                    WringController.activateSelected()
                }
            }
        }

        onItemSelected: function(index) {
            WringController.selectByIndex(index)
        }
    }

    Ring {
        id: ring2
        anchors.fill: parent
        visible: WringController.state === 2

        center: WringController.ring2Center
        radius: wringRoot.ring2Radius
        itemCount: {
            var ws = WringController.workspaces
            var apps = WringController.popularApps
            return (ws ? ws.length : 0) + (apps ? apps.length : 0)
        }
        selectedIndex: WringController.ring2SelectedIndex

        model: {
            var items = []
            var workspaces = WringController.workspaces
            var apps = WringController.popularApps

            for (var i = 0; i < workspaces.length; i++) {
                var ws = workspaces[i]
                items.push({
                    type: "workspace",
                    title: ws.name || "Desktop " + (ws.index + 1),
                    applicationName: "",
                    icon: null,
                    isActive: ws.isActive || false,
                    workspaceIndex: ws.index || 0,
                    windowCount: ws.windowCount || 0
                })
            }

            for (var j = 0; j < apps.length; j++) {
                var app = apps[j]
                items.push({
                    type: "popularapp",
                    title: app.displayName || app.name,
                    applicationName: app.name || "",
                    executable: app.executable || "",
                    icon: app.icon || null,
                    isActive: false
                })
            }

            return items
        }

        delegate: RingItem {
            width: wringRoot.itemSize
            height: wringRoot.itemSize

            itemData: modelData
            isSelected: index === WringController.ring2SelectedIndex
            ringCenter: WringController.ring2Center
            ringRadius: wringRoot.ring2Radius
            itemIndex: index
            totalItems: ring2.itemCount

            onClicked: {
                WringController.ring2SelectByIndex(index)
                WringController.activateRing2Item()
            }
        }

        onItemSelected: function(index) {
            WringController.ring2SelectByIndex(index)
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            WringController.hide()
        }
    }
}
