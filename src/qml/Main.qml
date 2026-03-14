import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

/**
 * Main.qml - Root application window for Aero audio recorder.
 * Layout: recording list sidebar on left, main recorder panel on right.
 * Design tokens are consistent with the amp++ music player.
 */
ApplicationWindow {
    id: root

    // ======== Design Tokens (matching amp++ music player) ========
    readonly property color bgColor: "#0F0F1A"
    readonly property color surfaceColor: "#1A1A2E"
    readonly property color surfaceLightColor: "#252542"
    readonly property color surfaceHoverColor: "#30304D"
    readonly property color accentColor: "#7C3AED"
    readonly property color accentLightColor: "#A78BFA"
    readonly property color accentDarkColor: "#5B21B6"
    readonly property color textColor: "#E8E8F0"
    readonly property color textSecondaryColor: "#9CA3AF"
    readonly property color textMutedColor: "#6B7280"
    readonly property color dividerColor: "#2D2D4A"
    readonly property color dangerColor: "#EF4444"
    readonly property color recordingColor: "#DC2626"
    readonly property color recordingGlowColor: "#EF4444"

    width: 1100
    height: 700
    minimumWidth: 850
    minimumHeight: 550
    visible: true
    title: qsTr("aero")
    color: bgColor

    // ======== Folder Dialog for output directory ========
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Output Directory")
        onAccepted: recorderEngine.outputDirectory = selectedFolder.toString().replace("file://", "")
    }

    // ======== Main Layout ========
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left sidebar: recordings list
        RecordingSidebar {
            id: recordingSidebar
            Layout.preferredWidth: 300
            Layout.fillHeight: true
        }

        // Divider
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: dividerColor
        }

        // Right panel: main recorder
        RecorderPanel {
            id: recorderPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            onOpenFolderDialog: folderDialog.open()
        }
    }

    // ======== Keyboard Shortcuts ========
    Shortcut { sequence: "Space"; onActivated: recorderEngine.toggleRecording() }
    Shortcut { sequence: "Escape"; onActivated: recorderEngine.stopPlayback() }
    Shortcut { sequence: "Ctrl+O"; onActivated: folderDialog.open() }

    // ======== Error Handling ========
    Connections {
        target: recorderEngine
        function onErrorOccurred(message) {
            errorPopup.text = message
            errorPopup.open()
        }
    }

    // Error popup
    Popup {
        id: errorPopup
        property string text: ""

        anchors.centerIn: parent
        width: 400
        height: errorContent.implicitHeight + 40
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: root.surfaceColor
            radius: 12
            border.color: root.dangerColor
            border.width: 1
        }

        ColumnLayout {
            id: errorContent
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Text {
                text: qsTr("Error")
                font.pixelSize: 16
                font.bold: true
                color: root.dangerColor
            }

            Text {
                text: errorPopup.text
                font.pixelSize: 13
                color: root.textColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight
                onClicked: errorPopup.close()

                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? root.surfaceHoverColor : root.surfaceLightColor
                }
                contentItem: Text {
                    text: parent.text
                    color: root.textColor
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
