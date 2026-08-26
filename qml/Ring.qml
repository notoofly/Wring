import QtQuick

Item {
    id: ringRoot

    property var center: ({x: 0, y: 0})
    property real radius: 150
    property int itemCount: 0
    property int selectedIndex: -1
    property var model: []
    property real startAngle: -Math.PI / 2
    property real itemSize: 64

    signal itemClicked(int index)

    Repeater {
        id: repeater
        model: ringRoot.model

        Item {
            id: ringItem

            required property int index
            required property var modelData

            property real angle: {
                if (ringRoot.itemCount === 0) return 0
                var angleStep = (2 * Math.PI) / ringRoot.itemCount
                return ringRoot.startAngle + index * angleStep
            }

            property real itemX: ringRoot.center.x + Math.cos(angle) * ringRoot.radius
            property real itemY: ringRoot.center.y + Math.sin(angle) * ringRoot.radius

            x: itemX - ringRoot.itemSize / 2
            y: itemY - ringRoot.itemSize / 2
            width: ringRoot.itemSize
            height: ringRoot.itemSize

            property bool isSelected: index === ringRoot.selectedIndex

            scale: isSelected ? 1.15 : 1.0
            z: isSelected ? 10 : 1

            Behavior on scale {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutQuad
                }
            }

            Behavior on x {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutQuad
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutQuad
                }
            }

            Loader {
                id: ringItemLoader
                anchors.fill: parent
                sourceComponent: {
                    if (!modelData) return null

                    switch (modelData.type) {
                        case "window": return windowItemComponent
                        case "ring2button": return ring2ButtonComponent
                        case "workspace": return workspaceItemComponent
                        case "popularapp": return popularAppItemComponent
                        default: return null
                    }
                }

                property string itemTitle: modelData ? (modelData.title || "") : ""
                property var itemIcon: modelData ? (modelData.icon || null) : null
                property bool itemIsActive: modelData ? (modelData.isActive || false) : false
                property bool itemIsSelected: ringItem.isSelected

                Component {
                    id: windowItemComponent
                    WindowItem {
                        title: ringItemLoader.itemTitle
                        icon: ringItemLoader.itemIcon
                        isActive: ringItemLoader.itemIsActive
                        isSelected: ringItemLoader.itemIsSelected
                    }
                }

                Component {
                    id: ring2ButtonComponent
                    Ring2Button {
                        isSelected: ringItemLoader.itemIsSelected
                    }
                }

                Component {
                    id: workspaceItemComponent
                    WorkspaceItem {
                        title: ringItemLoader.itemTitle
                        isActive: ringItemLoader.itemIsActive
                        isSelected: ringItemLoader.itemIsSelected
                    }
                }

                Component {
                    id: popularAppItemComponent
                    PopularAppItem {
                        title: ringItemLoader.itemTitle
                        icon: ringItemLoader.itemIcon
                        isSelected: ringItemLoader.itemIsSelected
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: function(mouse) {
                    mouse.accepted = true
                    ringRoot.itemClicked(ringItem.index)
                }
            }
        }
    }
}
