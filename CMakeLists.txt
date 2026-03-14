cmake_minimum_required(VERSION 3.16)
project(aero VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    Multimedia
)

set(SOURCES
    src/main.cpp
    src/engine/AudioRecorderEngine.cpp
    src/engine/WaveformProvider.cpp
    src/models/RecordingInfo.cpp
    src/models/RecordingListModel.cpp
)

set(HEADERS
    src/engine/AudioRecorderEngine.h
    src/engine/WaveformProvider.h
    src/models/RecordingInfo.h
    src/models/RecordingListModel.h
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(${PROJECT_NAME}
    URI AeroApp
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/RecordingSidebar.qml
        src/qml/RecorderPanel.qml
        src/qml/WaveformVisualizer.qml
        src/qml/VUMeter.qml
        src/qml/RecordButton.qml
    RESOURCES
        resources/icons/record.svg
        resources/icons/stop.svg
        resources/icons/play.svg
        resources/icons/pause.svg
        resources/icons/delete.svg
        resources/icons/rename.svg
        resources/icons/folder-open.svg
        resources/icons/mic.svg
        resources/icons/refresh.svg
        resources/icons/format.svg
        resources/icons/timer.svg
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/models
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
