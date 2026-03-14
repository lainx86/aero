import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * RecordingSidebar.qml - Left sidebar showing the list of saved recordings.
 * Matches PlaylistSidebar styling from the amp++ music player.
 */
Item {
    id: sidebarRoot

    // Design tokens from root
    property color surfaceColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceColor : "#1A1A2E"
    property color surfaceLightColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceLightColor : "#252542"
    property color surfaceHoverColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceHoverColor : "#30304D"
    property color accentColor: ApplicationWindow.window ? ApplicationWindow.window.accentColor : "#7C3AED"
    property color accentDarkColor: ApplicationWindow.window ? ApplicationWindow.window.accentDarkColor : "#5B21B6"
    property color accentLightColor: ApplicationWindow.window ? ApplicationWindow.window.accentLightColor : "#A78BFA"
    property color textColor: ApplicationWindow.window ? ApplicationWindow.window.textColor : "#E8E8F0"
    property color textSecondaryColor: ApplicationWindow.window ? ApplicationWindow.window.textSecondaryColor : "#9CA3AF"
    property color textMutedColor: ApplicationWindow.window ? ApplicationWindow.window.textMutedColor : "#6B7280"
    property color dividerColor: ApplicationWindow.window ? ApplicationWindow.window.dividerColor : "#2D2D4A"
    property color dangerColor: ApplicationWindow.window ? ApplicationWindow.window.dangerColor : "#EF4444"

    Rectangle {
        anchors.fill: parent
        color: surfaceColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // ======== Header ========
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: surfaceColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 12

                    Text {
                        text: "aero"
                        font.pixelSize: 17
                        font.bold: true
                        color: sidebarRoot.textColor
                        Layout.fillWidth: true
                    }

                    // Recording count badge
                    Rectangle {
                        width: countLabel.implicitWidth + 12
                        height: 22
                        radius: 11
                        color: sidebarRoot.accentDarkColor

                        Text {
                            id: countLabel
                            anchors.centerIn: parent
                            text: recordingListModel.count
                            font.pixelSize: 11
                            font.bold: true
                            color: "#FFFFFF"
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: sidebarRoot.dividerColor
                }
            }

            // ======== Recordings List ========
            ListView {
                id: recordingsView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: recordingListModel

                ScrollBar.vertical: ScrollBar {
                    active: true
                    policy: ScrollBar.AsNeeded
                    contentItem: Rectangle {
                        implicitWidth: 4
                        radius: 2
                        color: sidebarRoot.textMutedColor
                        opacity: parent.active ? 0.7 : 0.2
                    }
                }

                // Empty placeholder
                Column {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: recordingListModel.count === 0
                    opacity: 0.5

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 48; height: 48
                        source: "qrc:/AeroApp/resources/icons/mic.svg"
                        opacity: 0.4
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("No recordings yet")
                        font.pixelSize: 14
                        color: sidebarRoot.textMutedColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Start recording to see files here")
                        font.pixelSize: 12
                        color: sidebarRoot.textMutedColor
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // ======== Delegate ========
                delegate: Item {
                    id: delRoot
                    width: recordingsView.width
                    height: 62

                    required property int index
                    required property string fileName
                    required property string formattedDuration
                    required property string formattedSize
                    required property string createdAt
                    required property bool isPlaying

                    // Inline rename state
                    property bool renaming: false

                    Rectangle {
                        id: delBg
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        anchors.topMargin: 2
                        anchors.bottomMargin: 2
                        radius: 10
                        color: {
                            if (delRoot.isPlaying)
                                return sidebarRoot.accentDarkColor
                            if (delMouse.containsMouse)
                                return sidebarRoot.surfaceHoverColor
                            return "transparent"
                        }

                        Behavior on color { ColorAnimation { duration: 150 } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 8
                            spacing: 10

                            // Play indicator / index number
                            Item {
                                Layout.preferredWidth: 26
                                Layout.preferredHeight: 26

                                Text {
                                    anchors.centerIn: parent
                                    text: (delRoot.index + 1)
                                    font.pixelSize: 11
                                    color: sidebarRoot.textMutedColor
                                    visible: !delRoot.isPlaying
                                }

                                // Equalizer bars (playing indicator)
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 2
                                    visible: delRoot.isPlaying

                                    Repeater {
                                        model: 3
                                        Rectangle {
                                            property real barIndex: index
                                            width: 3; height: 6
                                            color: sidebarRoot.accentLightColor
                                            radius: 1.5
                                            anchors.bottom: parent.bottom

                                            SequentialAnimation on height {
                                                running: delRoot.isPlaying && recorderEngine.playing
                                                loops: Animation.Infinite
                                                NumberAnimation { to: 8 + barIndex * 4; duration: 250 + barIndex * 80; easing.type: Easing.InOutQuad }
                                                NumberAnimation { to: 4; duration: 200 + barIndex * 60; easing.type: Easing.InOutQuad }
                                            }
                                        }
                                    }
                                }
                            }

                            // File info
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                // File name (or rename input)
                                Loader {
                                    Layout.fillWidth: true
                                    sourceComponent: delRoot.renaming ? renameComponent : nameComponent
                                }

                                Component {
                                    id: nameComponent
                                    Text {
                                        text: delRoot.fileName
                                        font.pixelSize: 13
                                        font.bold: delRoot.isPlaying
                                        color: delRoot.isPlaying ? "#FFFFFF" : sidebarRoot.textColor
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                    }
                                }

                                Component {
                                    id: renameComponent
                                    TextField {
                                        id: renameField
                                        text: delRoot.fileName
                                        font.pixelSize: 13
                                        color: sidebarRoot.textColor
                                        selectionColor: sidebarRoot.accentColor
                                        selectedTextColor: "#FFFFFF"
                                        background: Rectangle {
                                            color: sidebarRoot.surfaceLightColor
                                            radius: 4
                                            border.color: sidebarRoot.accentColor
                                            border.width: 1
                                        }
                                        Component.onCompleted: { selectAll(); forceActiveFocus() }
                                        onAccepted: {
                                            recordingListModel.renameRecording(delRoot.index, text)
                                            delRoot.renaming = false
                                        }
                                        Keys.onEscapePressed: delRoot.renaming = false
                                    }
                                }

                                // Duration + size
                                RowLayout {
                                    spacing: 8
                                    Text {
                                        text: delRoot.formattedDuration
                                        font.pixelSize: 11
                                        color: delRoot.isPlaying
                                            ? sidebarRoot.accentLightColor
                                            : sidebarRoot.textSecondaryColor
                                    }
                                    Text {
                                        text: "·"
                                        font.pixelSize: 11
                                        color: sidebarRoot.textMutedColor
                                    }
                                    Text {
                                        text: delRoot.formattedSize
                                        font.pixelSize: 11
                                        color: sidebarRoot.textSecondaryColor
                                    }
                                }
                            }

                            // Action buttons (visible on hover or playing)
                            Row {
                                spacing: 2
                                visible: delMouse.containsMouse || delRoot.isPlaying

                                // Rename button
                                RoundButton {
                                    width: 28; height: 28; flat: true
                                    icon.source: "qrc:/AeroApp/resources/icons/rename.svg"
                                    icon.width: 12; icon.height: 12
                                    icon.color: sidebarRoot.textMutedColor
                                    onClicked: delRoot.renaming = true
                                    scale: pressed ? 0.8 : 1.0
                                    Behavior on scale { NumberAnimation { duration: 100 } }
                                }

                                // Delete button
                                RoundButton {
                                    width: 28; height: 28; flat: true
                                    icon.source: "qrc:/AeroApp/resources/icons/delete.svg"
                                    icon.width: 12; icon.height: 12
                                    icon.color: sidebarRoot.dangerColor
                                    onClicked: recordingListModel.deleteRecording(delRoot.index)
                                    scale: pressed ? 0.8 : 1.0
                                    Behavior on scale { NumberAnimation { duration: 100 } }
                                }
                            }
                        }

                        MouseArea {
                            id: delMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                if (delRoot.isPlaying)
                                    recorderEngine.stopPlayback()
                                else
                                    recorderEngine.playRecordingAt(delRoot.index)
                            }
                        }
                    }
                }

                // List animations
                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250 }
                    NumberAnimation { property: "scale"; from: 0.85; to: 1; duration: 250 }
                }
                remove: Transition {
                    NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150 }
                }
                displaced: Transition {
                    NumberAnimation { properties: "y"; duration: 250 }
                }
            }

            // ======== Footer ========
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: sidebarRoot.dividerColor
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                color: sidebarRoot.surfaceColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10

                    // Open folder button
                    RoundButton {
                        width: 36; height: 36; flat: true
                        icon.source: "qrc:/AeroApp/resources/icons/folder-open.svg"
                        icon.width: 18; icon.height: 18
                        icon.color: sidebarRoot.accentLightColor
                        onClicked: recorderEngine.openOutputDirectory()
                        ToolTip.visible: hovered; ToolTip.text: qsTr("Open Folder"); ToolTip.delay: 400
                        scale: pressed ? 0.85 : (hovered ? 1.1 : 1.0)
                        Behavior on scale { NumberAnimation { duration: 120 } }
                    }

                    // Refresh button
                    RoundButton {
                        width: 36; height: 36; flat: true
                        icon.source: "qrc:/AeroApp/resources/icons/refresh.svg"
                        icon.width: 18; icon.height: 18
                        icon.color: sidebarRoot.textSecondaryColor
                        onClicked: recordingListModel.refresh()
                        ToolTip.visible: hovered; ToolTip.text: qsTr("Refresh"); ToolTip.delay: 400
                        scale: pressed ? 0.85 : (hovered ? 1.1 : 1.0)
                        Behavior on scale { NumberAnimation { duration: 120 } }
                    }

                    Item { Layout.fillWidth: true }

                    // Output directory display
                    Text {
                        text: recorderEngine.outputDirectory.split("/").pop()
                        font.pixelSize: 11
                        color: sidebarRoot.textMutedColor
                        elide: Text.ElideLeft
                        Layout.maximumWidth: 100
                    }
                }
            }
        }
    }
}
