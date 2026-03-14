import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * VUMeter.qml - Horizontal audio level meter with green-yellow-red gradient.
 * Tracks waveformProvider.currentLevel with smooth transitions.
 */
Item {
    id: vuRoot

    property real level: waveformProvider.currentLevel
    property color accentColor: ApplicationWindow.window ? ApplicationWindow.window.accentColor : "#7C3AED"
    property color surfaceLightColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceLightColor : "#252542"
    property color textMutedColor: ApplicationWindow.window ? ApplicationWindow.window.textMutedColor : "#6B7280"
    property color textSecondaryColor: ApplicationWindow.window ? ApplicationWindow.window.textSecondaryColor : "#9CA3AF"

    implicitWidth: 400
    implicitHeight: 28

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // Label
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("Level")
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1
                color: vuRoot.textMutedColor
            }
            Item { Layout.fillWidth: true }
            Text {
                text: Math.round(vuRoot.level * 100) + "%"
                font.pixelSize: 10
                color: vuRoot.textSecondaryColor
            }
        }

        // Meter bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 8
            radius: 4
            color: vuRoot.surfaceLightColor

            // Level fill
            Rectangle {
                id: levelFill
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width * vuRoot.level
                radius: 4

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: "#22C55E" }   // Green
                    GradientStop { position: 0.5; color: "#EAB308" }   // Yellow
                    GradientStop { position: 0.85; color: "#F97316" }  // Orange
                    GradientStop { position: 1.0; color: "#EF4444" }   // Red
                }

                Behavior on width {
                    NumberAnimation { duration: 60; easing.type: Easing.OutQuad }
                }
            }

            // Peak markers
            Repeater {
                model: 10
                Rectangle {
                    x: parent.width * ((index + 1) / 10.0) - 0.5
                    width: 1
                    height: parent.height
                    color: Qt.rgba(0, 0, 0, 0.3)
                }
            }
        }
    }
}
