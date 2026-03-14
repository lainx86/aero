# aero

A modern audio recorder desktop application built with C++23 and Qt 6.

## Features

- Record audio from microphone in WAV, MP3, or OGG format
- Real-time waveform visualization during recording
- VU meter with color-coded level display
- Recording timer with optional max-duration auto-stop
- Recording list with playback, rename, and delete
- Automatic timestamp-based file naming
- Dark theme UI consistent with the amp++ music player

## Dependencies

Install the required packages on Arch Linux:

```bash
sudo pacman -S qt6-base qt6-declarative qt6-multimedia qt6-multimedia-ffmpeg cmake gcc
```

## Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## Run

```bash
./build/aero
```

Recordings are saved to `~/Music/aero/` by default.

## Project Structure

```
src/
  main.cpp                    Application entry point
  engine/
    AudioRecorderEngine.h/cpp Audio recording and playback engine
    WaveformProvider.h/cpp    Real-time audio level data provider
  models/
    RecordingInfo.h/cpp       Recording metadata data structure
    RecordingListModel.h/cpp  List model for QML recordings view
  qml/
    Main.qml                  Root application window
    RecordingSidebar.qml      Left sidebar with recordings list
    RecorderPanel.qml         Main recording control panel
    WaveformVisualizer.qml    Canvas-based waveform display
    VUMeter.qml               Audio level meter
    RecordButton.qml          Animated record/stop button
resources/
  icons/                      SVG icons
```

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Space    | Toggle recording (start/stop) |
| Escape   | Stop playback |
| Ctrl+O   | Change output directory |

## License

MIT
