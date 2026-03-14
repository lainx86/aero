import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * RecorderPanel.qml - Main recording area on the right side.
 * Contains timer, waveform visualizer, VU meter, record button,
 * format selector, and settings.
 */
Item {
    id: panelRoot

    signal openFolderDialog()

    // Design tokens
    property color bgColor: ApplicationWindow.window ? ApplicationWindow.window.bgColor : "#0F0F1A"
    property color surfaceColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceColor : "#1A1A2E"
    property color surfaceLightColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceLightColor : "#252542"
    property color surfaceHoverColor: ApplicationWindow.window ? ApplicationWindow.window.surfaceHoverColor : "#30304D"
    property color accentColor: ApplicationWindow.window ? ApplicationWindow.window.accentColor : "#7C3AED"
    property color accentLightColor: ApplicationWindow.window ? ApplicationWindow.window.accentLightColor : "#A78BFA"
    property color accentDarkColor: ApplicationWindow.window ? ApplicationWindow.window.accentDarkColor : "#5B21B6"
    property color textColor: ApplicationWindow.window ? ApplicationWindow.window.textColor : "#E8E8F0"
    property color textSecondaryColor: ApplicationWindow.window ? ApplicationWindow.window.textSecondaryColor : "#9CA3AF"
    property color textMutedColor: ApplicationWindow.window ? ApplicationWindow.window.textMutedColor : "#6B7280"
    property color dividerColor: ApplicationWindow.window ? ApplicationWindow.window.dividerColor : "#2D2D4A"
    property color dangerColor: ApplicationWindow.window ? ApplicationWindow.window.dangerColor : "#EF4444"
    property color recordingColor: ApplicationWindow.window ? ApplicationWindow.window.recordingColor : "#DC2626"

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        // Background gradient (matching music player)
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(0.122, 0.067, 0.231, 0.5) }
                GradientStop { position: 0.5; color: panelRoot.bgColor }
                GradientStop { position: 1.0; color: panelRoot.bgColor }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16

            // ======== Top spacer ========
            Item { Layout.fillHeight: true; Layout.maximumHeight: 40 }

            // ======== Status label ========
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: {
                    if (recorderEngine.recording) return qsTr("Recording...")
                    if (recorderEngine.paused) return qsTr("Paused")
                    if (recorderEngine.playing) return qsTr("Playing")
                    return qsTr("Ready to Record")
                }
                font.pixelSize: 13
                font.bold: true
                font.letterSpacing: 2
                color: {
                    if (recorderEngine.recording) return panelRoot.recordingColor
                    if (recorderEngine.playing) return panelRoot.accentLightColor
                    return panelRoot.textMutedColor
                }

                // Blink when recording
                SequentialAnimation on opacity {
                    running: recorderEngine.recording
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.4; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
                // Reset opacity when not recording
                NumberAnimation on opacity {
                    running: !recorderEngine.recording
                    to: 1.0; duration: 200
                }
            }

            // ======== Timer display ========
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: recorderEngine.elapsedTimeFormatted
                font.pixelSize: 64
                font.bold: true
                font.family: "monospace"
                color: {
                    if (recorderEngine.recording) return panelRoot.textColor
                    return panelRoot.textSecondaryColor
                }
                Behavior on color { ColorAnimation { duration: 200 } }
            }

            // ======== Max duration indicator ========
            Text {
                Layout.alignment: Qt.AlignHCenter
                visible: recorderEngine.maxDuration > 0
                text: {
                    var max = recorderEngine.maxDuration
                    var mins = Math.floor(max / 60)
                    var secs = max % 60
                    return qsTr("Max: %1:%2").arg(String(mins).padStart(2, '0')).arg(String(secs).padStart(2, '0'))
                }
                font.pixelSize: 12
                color: panelRoot.textMutedColor
            }

            // ======== Waveform Visualizer ========
            WaveformVisualizer {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                Layout.maximumWidth: 600
                Layout.alignment: Qt.AlignHCenter
            }

            // ======== VU Meter ========
            VUMeter {
                Layout.fillWidth: true
                Layout.maximumWidth: 500
                Layout.alignment: Qt.AlignHCenter
            }

            // ======== Spacer ========
            Item { Layout.preferredHeight: 8 }

            // ======== Record Button ========
            RecordButton {
                Layout.alignment: Qt.AlignHCenter
                onClicked: recorderEngine.toggleRecording()
            }

            // ======== Spacer ========
            Item { Layout.fillHeight: true }

            // ======== Input Device Selector ========
            RowLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: 600
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                Image {
                    source: "qrc:/AeroApp/resources/icons/mic.svg"
                    sourceSize: Qt.size(14, 14)
                    opacity: 0.5
                }

                Text {
                    text: qsTr("Input:")
                    font.pixelSize: 12
                    color: panelRoot.textMutedColor
                }

                ComboBox {
                    id: deviceCombo
                    Layout.fillWidth: true
                    model: recorderEngine.inputDevices
                    textRole: "deviceName"
                    currentIndex: recorderEngine.currentInputDeviceIndex
                    
                    onActivated: (index) => {
                        recorderEngine.currentInputDeviceIndex = index
                    }

                    background: Rectangle {
                        color: panelRoot.surfaceLightColor
                        border.color: panelRoot.dividerColor
                        border.width: 1
                        radius: 6
                    }
                    contentItem: Text {
                        text: deviceCombo.displayText
                        color: panelRoot.textColor
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: 8
                        rightPadding: 24
                    }
                    indicator: Text {
                        x: deviceCombo.width - width - 8
                        y: (deviceCombo.height - height) / 2
                        text: "▼"
                        color: panelRoot.textMutedColor
                        font.pixelSize: 10
                    }
                    popup: Popup {
                        y: deviceCombo.height - 1
                        width: deviceCombo.width
                        implicitHeight: contentItem.implicitHeight
                        padding: 1
                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: deviceCombo.popup.visible ? deviceCombo.delegateModel : null
                            currentIndex: deviceCombo.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }
                        background: Rectangle {
                            color: panelRoot.surfaceColor
                            border.color: panelRoot.dividerColor
                            radius: 6
                        }
                    }
                    delegate: ItemDelegate {
                        width: deviceCombo.width
                        contentItem: Text {
                            text: model.deviceName
                            color: highlighted ? panelRoot.accentLightColor : panelRoot.textColor
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: highlighted ? panelRoot.surfaceHoverColor : "transparent"
                        }
                    }
                }
            }

            // ======== Bottom Controls ========
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: panelRoot.dividerColor
                opacity: 0.5
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: 600
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                // Format selector
                RowLayout {
                    spacing: 6

                    Image {
                        source: "qrc:/AeroApp/resources/icons/format.svg"
                        sourceSize: Qt.size(14, 14)
                        opacity: 0.5
                    }

                    Text {
                        text: qsTr("Format:")
                        font.pixelSize: 12
                        color: panelRoot.textMutedColor
                    }

                    // Format buttons
                    Repeater {
                        model: ["WAV", "MP3", "OGG"]
                        delegate: RoundButton {
                            required property int index
                            required property string modelData
                            text: modelData
                            width: 50; height: 28
                            flat: true
                            font.pixelSize: 11
                            font.bold: recorderEngine.outputFormat === index

                            background: Rectangle {
                                radius: 6
                                color: recorderEngine.outputFormat === index
                                    ? panelRoot.accentDarkColor
                                    : (parent.hovered ? panelRoot.surfaceHoverColor : "transparent")
                                Behavior on color { ColorAnimation { duration: 150 } }
                            }

                            contentItem: Text {
                                text: modelData
                                color: recorderEngine.outputFormat === index
                                    ? "#FFFFFF"
                                    : panelRoot.textSecondaryColor
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: recorderEngine.outputFormat = index
                            scale: pressed ? 0.9 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }
                    }
                }

                // Separator
                Rectangle { width: 1; height: 20; color: panelRoot.dividerColor; opacity: 0.5 }

                // Max duration setting
                RowLayout {
                    spacing: 6

                    Image {
                        source: "qrc:/AeroApp/resources/icons/timer.svg"
                        sourceSize: Qt.size(14, 14)
                        opacity: 0.5
                    }

                    Text {
                        text: qsTr("Limit:")
                        font.pixelSize: 12
                        color: panelRoot.textMutedColor
                    }

                    SpinBox {
                        id: maxDurationSpin
                        from: 0; to: 3600; stepSize: 30
                        value: recorderEngine.maxDuration
                        onValueModified: recorderEngine.maxDuration = value

                        width: 100; height: 28

                        contentItem: Text {
                            text: {
                                if (maxDurationSpin.value === 0) return "Off"
                                var m = Math.floor(maxDurationSpin.value / 60)
                                var s = maxDurationSpin.value % 60
                                return m + ":" + String(s).padStart(2, '0')
                            }
                            font.pixelSize: 12
                            color: panelRoot.textColor
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: 6
                            color: panelRoot.surfaceLightColor
                            border.color: panelRoot.dividerColor
                            border.width: 1
                        }

                        up.indicator: Rectangle {
                            x: parent.width - width - 2
                            y: 2; width: 20; height: parent.height - 4
                            radius: 4
                            color: maxDurationSpin.up.hovered ? panelRoot.surfaceHoverColor : "transparent"
                            Text { anchors.centerIn: parent; text: "+"; color: panelRoot.textColor; font.pixelSize: 14 }
                        }

                        down.indicator: Rectangle {
                            x: 2; y: 2; width: 20; height: parent.height - 4
                            radius: 4
                            color: maxDurationSpin.down.hovered ? panelRoot.surfaceHoverColor : "transparent"
                            Text { anchors.centerIn: parent; text: "-"; color: panelRoot.textColor; font.pixelSize: 14 }
                        }
                    }
                }

                // Separator
                Rectangle { width: 1; height: 20; color: panelRoot.dividerColor; opacity: 0.5 }

                // Output directory
                RoundButton {
                    width: 30; height: 28; flat: true
                    icon.source: "qrc:/AeroApp/resources/icons/folder-open.svg"
                    icon.width: 14; icon.height: 14
                    icon.color: panelRoot.textSecondaryColor
                    onClicked: panelRoot.openFolderDialog()
                    ToolTip.visible: hovered; ToolTip.text: qsTr("Change Output Folder"); ToolTip.delay: 400
                    scale: pressed ? 0.85 : (hovered ? 1.1 : 1.0)
                    Behavior on scale { NumberAnimation { duration: 120 } }
                }
            }

            // Bottom padding
            Item { Layout.preferredHeight: 8 }
        }
    }
}
