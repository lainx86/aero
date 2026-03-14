#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "engine/AudioRecorderEngine.h"

/**
 * @brief Main entry point for the Aero audio recorder application.
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("aero"));
    app.setOrganizationName(QStringLiteral("aero"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    // Use Basic style for full custom theming control
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    // Create the audio recorder engine singleton
    AudioRecorderEngine recorderEngine;

    // Setup QML engine
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty(
        QStringLiteral("recorderEngine"), &recorderEngine);
    engine.rootContext()->setContextProperty(
        QStringLiteral("waveformProvider"), recorderEngine.waveform());
    engine.rootContext()->setContextProperty(
        QStringLiteral("recordingListModel"), recorderEngine.recordings());

    // Load main QML file
    const QUrl mainQml(QStringLiteral("qrc:/AeroApp/src/qml/Main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() {
            qCritical() << "[Main] Failed to load QML interface!";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(mainQml);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "[Main] No root objects loaded!";
        return -1;
    }

    qDebug() << "[Main] aero started successfully!";
    return app.exec();
}
