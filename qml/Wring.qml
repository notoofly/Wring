import QtQuick
import QtQuick.Window
import Wring 1.0

Item {
    id: wringRoot

    property real ring1Radius: 150
    property real ring2Radius: 120
    property real itemSize: 64
    property int animationDuration: 200
    property bool showingSettings: false

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
            showingSettings = false
            wringRoot.opacity = 0.0
            wringRoot.scale = 0.8
        }
    }

    // Settings page (overlay)
    SettingsPage {
        id: settingsPage
        anchors.fill: parent
        visible: showingSettings

        onBack: {
            showingSettings = false
        }
    }

    Ring {
        id: ring1
        anchors.fill: parent
        visible: WringController.state === 1 && !showingSettings

        center: WringController.ring1Center
        radius: wringRoot.ring1Radius
        itemCount: WringController.ring1ItemCount
        selectedIndex: WringController.selectedIndex
        itemSize: wringRoot.itemSize

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

        onItemClicked: function(index) {
            var items = ring1.model
            if (index < 0 || index >= items.length) return
            var item = items[index]
            if (item.type === "ring2button") {
                WringController.openRing2()
            } else {
                WringController.selectByIndex(index)
                WringController.activateSelected()
            }
        }
    }

    Ring {
        id: ring2
        anchors.fill: parent
        visible: WringController.state === 2 && !showingSettings

        center: WringController.ring2Center
        radius: wringRoot.ring2Radius
        itemCount: {
            var ws = WringController.workspaces
            var apps = WringController.popularApps
            return (ws ? ws.length : 0) + (apps ? apps.length : 0) + 1
        }
        selectedIndex: WringController.ring2SelectedIndex
        itemSize: wringRoot.itemSize

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

            items.push({
                type: "settings",
                title: "Settings",
                applicationName: "",
                icon: null,
                isActive: false,
                id: -2
            })

            return items
        }

        onItemClicked: function(index) {
            var items = ring2.model
            if (index < 0 || index >= items.length) return
            var item = items[index]
            if (item.type === "settings") {
                showingSettings = true
            } else {
                WringController.ring2SelectByIndex(index)
                WringController.activateRing2Item()
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        visible: !showingSettings
        onClicked: function(mouse) {
            WringController.hide()
        }
    }
}
