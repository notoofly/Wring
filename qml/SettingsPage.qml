import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Wring 1.0

Rectangle {
    id: settingsPage

    color: "#1a1a2e"
    radius: 12

    signal back()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Settings"
                color: "#ffffff"
                font.pixelSize: 22
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 32
                height: 32
                radius: 16
                color: closeArea.containsMouse ? "#ff4444" : "#444444"

                Text {
                    anchors.centerIn: parent
                    text: "\u00D7"
                    color: "#ffffff"
                    font.pixelSize: 18
                }

                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: settingsPage.back()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333355"
        }

        // Trigger Modifier — checkboxes for combining
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Trigger Modifier (combine freely)"
                color: "#aaaacc"
                font.pixelSize: 14
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 8
                rowSpacing: 8

                Repeater {
                    model: [
                        { label: "Super", mask: WringSettings.Super },
                        { label: "Ctrl",  mask: WringSettings.Ctrl },
                        { label: "Alt",   mask: WringSettings.Alt },
                        { label: "Shift", mask: WringSettings.Shift }
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: 6
                        color: WringSettings.hasModifier(modelData.mask) ? "#4466aa" : "#2a2a4a"
                        border.color: WringSettings.hasModifier(modelData.mask) ? "#6688cc" : "#3a3a5a"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            Rectangle {
                                width: 20
                                height: 20
                                radius: 4
                                color: WringSettings.hasModifier(modelData.mask) ? "#6688cc" : "#1a1a2e"
                                border.color: "#6688cc"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: WringSettings.hasModifier(modelData.mask) ? "\u2713" : ""
                                    color: "#ffffff"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                            }

                            Text {
                                text: modelData.label
                                color: "#ffffff"
                                font.pixelSize: 14
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: WringSettings.toggleModifier(modelData.mask)
                        }
                    }
                }
            }
        }

        // Trigger Button
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Trigger Button"
                color: "#aaaacc"
                font.pixelSize: 14
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Repeater {
                    model: [
                        { label: "Left",   value: WringSettings.Left },
                        { label: "Middle", value: WringSettings.Middle },
                        { label: "Right",  value: WringSettings.Right }
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: 6
                        color: modelData.value === WringSettings.triggerButton ? "#4466aa" : "#2a2a4a"
                        border.color: modelData.value === WringSettings.triggerButton ? "#6688cc" : "#3a3a5a"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: modelData.label
                            color: "#ffffff"
                            font.pixelSize: 14
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: WringSettings.triggerButton = modelData.value
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333355"
        }

        // Current shortcut display
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Current Shortcut"
                color: "#aaaacc"
                font.pixelSize: 14
            }

            Rectangle {
                Layout.fillWidth: true
                height: 48
                radius: 8
                color: "#2a2a4a"
                border.color: "#ff8800"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: WringSettings.shortcutString()
                    color: "#ff8800"
                    font.pixelSize: 18
                    font.bold: true
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
