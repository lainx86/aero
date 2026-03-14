import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * RecordButton.qml - Large animated record button.
 * Accent-colored when idle, red with pulsing glow when recording.
 * Matches the play button style from amp++ music player.
 */
Item {
    id: btnRoot

    property bool isRecording: recorderEngine.recording
    property bool isPaused: recorderEngine.paused
    property color accentColor: ApplicationWindow.window ? ApplicationWindow.window.accentColor : "#7C3AED"
    property color dangerColor: ApplicationWindow.window ? ApplicationWindow.window.dangerColor : "#EF4444"
    property color recordingColor: ApplicationWindow.window ? ApplicationWindow.window.recordingColor : "#DC2626"

    signal clicked()

    implicitWidth: 80
    implicitHeight: 80

    // Outer glow ring
    Rectangle {
        id: glowRing
        anchors.centerIn: parent
        width: 92
        height: 92
        radius: width / 2
        color: "transparent"
        border.color: isRecording
            ? Qt.rgba(0.863, 0.149, 0.149, 0.4)
            : Qt.rgba(0.486, 0.227, 0.929, 0.15)
        border.width: 2

        Behavior on border.color { ColorAnimation { duration: 300 } }

        // Pulsing animation when recording
        SequentialAnimation on scale {
            running: isRecording
            loops: Animation.Infinite
            NumberAnimation { to: 1.15; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
        }

        // Reset scale when not recording
        NumberAnimation on scale {
            running: !isRecording
            to: 1.0; duration: 200
        }
    }

    // Second glow ring (recording only)
    Rectangle {
        anchors.centerIn: parent
        width: 104
        height: 104
        radius: width / 2
        color: "transparent"
        border.color: Qt.rgba(0.863, 0.149, 0.149, 0.2)
        border.width: 1.5
        visible: isRecording
        opacity: isRecording ? 1.0 : 0.0

        Behavior on opacity { NumberAnimation { duration: 300 } }

        SequentialAnimation on scale {
            running: isRecording
            loops: Animation.Infinite
            NumberAnimation { to: 1.2; duration: 1000; easing.type: Easing.InOutQuad }
            NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
        }
    }

    // Main button
    RoundButton {
        id: mainBtn
        anchors.centerIn: parent
        width: 72; height: 72

        background: Rectangle {
            radius: 36
            color: {
                if (isRecording) return btnRoot.recordingColor
                if (isPaused) return Qt.darker(btnRoot.recordingColor, 1.3)
                return btnRoot.accentColor
            }
            border.color: {
                if (isRecording) return Qt.lighter(btnRoot.recordingColor, 1.3)
                return Qt.lighter(btnRoot.accentColor, 1.3)
            }
            border.width: mainBtn.hovered ? 2 : 0

            Behavior on color { ColorAnimation { duration: 250 } }
            Behavior on border.width { NumberAnimation { duration: 150 } }
        }

        // Icon: circle when idle (record), square when recording (stop)
        contentItem: Item {
            // Record icon (circle)
            Rectangle {
                anchors.centerIn: parent
                width: 24; height: 24
                radius: 12
                color: "#FFFFFF"
                visible: !isRecording && !isPaused

                Behavior on width { NumberAnimation { duration: 200 } }
            }

            // Stop icon (rounded square)
            Rectangle {
                anchors.centerIn: parent
                width: 22; height: 22
                radius: 4
                color: "#FFFFFF"
                visible: isRecording || isPaused
            }
        }

        onClicked: btnRoot.clicked()

        ToolTip.visible: hovered
        ToolTip.text: {
            if (isRecording) return qsTr("Stop Recording")
            if (isPaused) return qsTr("Resume Recording")
            return qsTr("Start Recording")
        }
        ToolTip.delay: 400

        scale: pressed ? 0.9 : (hovered ? 1.06 : 1.0)
        Behavior on scale { NumberAnimation { duration: 120 } }
    }
}
