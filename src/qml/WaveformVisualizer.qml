import QtQuick
import QtQuick.Controls

/**
 * WaveformVisualizer.qml - Canvas-based real-time waveform display.
 * Draws accent-colored bars from waveformProvider.samples.
 * Shows a subtle idle animation when not recording.
 */
Item {
    id: waveRoot

    property color accentColor: ApplicationWindow.window ? ApplicationWindow.window.accentColor : "#7C3AED"
    property color accentLightColor: ApplicationWindow.window ? ApplicationWindow.window.accentLightColor : "#A78BFA"
    property color accentDarkColor: ApplicationWindow.window ? ApplicationWindow.window.accentDarkColor : "#5B21B6"
    property color surfaceLightColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceLightColor : "#252542"
    property bool isActive: recorderEngine.recording

    implicitWidth: 500
    implicitHeight: 160

    // Background
    Rectangle {
        anchors.fill: parent
        radius: 12
        color: waveRoot.surfaceLightColor
        opacity: 0.4
    }

    // Waveform canvas
    Canvas {
        id: waveCanvas
        anchors.fill: parent
        anchors.margins: 8

        property var samples: waveformProvider.samples
        property real animOffset: 0

        onSamplesChanged: requestPaint()

        // Idle animation timer
        Timer {
            id: idleTimer
            interval: 40
            running: !waveRoot.isActive
            repeat: true
            onTriggered: {
                waveCanvas.animOffset += 0.05
                waveCanvas.requestPaint()
            }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var sampleList = samples
            var barCount = sampleList.length
            if (barCount === 0) return

            var barWidth = Math.max(2, (width / barCount) - 1)
            var gap = 1
            var centerY = height / 2

            for (var i = 0; i < barCount; i++) {
                var level = sampleList[i]

                // If idle, generate a subtle sine wave
                if (!waveRoot.isActive && level < 0.01) {
                    level = 0.03 + Math.sin((i * 0.15) + animOffset) * 0.02
                }

                var barHeight = Math.max(2, level * (height - 4))
                var x = i * (barWidth + gap)
                var y = centerY - barHeight / 2

                // Gradient color based on level
                var r, g, b
                if (level < 0.5) {
                    // Accent dark to accent
                    var t = level * 2
                    r = lerp(0.357, 0.486, t)
                    g = lerp(0.129, 0.227, t)
                    b = lerp(0.714, 0.929, t)
                } else {
                    // Accent to accent light
                    var t2 = (level - 0.5) * 2
                    r = lerp(0.486, 0.655, t2)
                    g = lerp(0.227, 0.545, t2)
                    b = lerp(0.929, 0.980, t2)
                }

                ctx.fillStyle = Qt.rgba(r, g, b, 0.7 + level * 0.3)
                ctx.beginPath()
                ctx.roundedRect(x, y, barWidth, barHeight, barWidth / 2, barWidth / 2)
                ctx.fill()
            }
        }

        function lerp(a, b, t) {
            return a + (b - a) * t
        }
    }

    // Center line
    Rectangle {
        anchors.centerIn: parent
        width: parent.width - 16
        height: 1
        color: waveRoot.accentDarkColor
        opacity: 0.3
    }
}
