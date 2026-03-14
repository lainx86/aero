#ifndef AUDIORECORDERENGINE_H
#define AUDIORECORDERENGINE_H

#include <QObject>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QAudioInput>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaFormat>
#include <QTimer>
#include <QUrl>

class QAudioSource;

#include "WaveformProvider.h"
#include "models/RecordingListModel.h"

/**
 * @brief Core audio recorder engine wrapping Qt6 Multimedia APIs.
 *
 * Handles recording from microphone, playback of recordings,
 * format selection, timer, and max-duration auto-stop.
 * All properties are exposed to QML via Q_PROPERTY.
 */
class AudioRecorderEngine : public QObject
{
    Q_OBJECT

    // Recording state
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY recordingStateChanged)

    // Playback state
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 playbackPosition READ playbackPosition NOTIFY playbackPositionChanged)
    Q_PROPERTY(qint64 playbackDuration READ playbackDuration NOTIFY playbackDurationChanged)

    // Timer
    Q_PROPERTY(qint64 elapsedTime READ elapsedTime NOTIFY elapsedTimeChanged)
    Q_PROPERTY(QString elapsedTimeFormatted READ elapsedTimeFormatted NOTIFY elapsedTimeChanged)

    // Settings
    Q_PROPERTY(int outputFormat READ outputFormat WRITE setOutputFormat NOTIFY outputFormatChanged)
    Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory NOTIFY outputDirectoryChanged)
    Q_PROPERTY(int maxDuration READ maxDuration WRITE setMaxDuration NOTIFY maxDurationChanged)

    // Sub-components
    Q_PROPERTY(WaveformProvider* waveform READ waveform CONSTANT)
    Q_PROPERTY(RecordingListModel* recordings READ recordings CONSTANT)

    // Error
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    /**
     * @brief Supported output audio formats.
     */
    enum OutputFormat {
        FormatWAV = 0,
        FormatMP3 = 1,
        FormatOGG = 2
    };
    Q_ENUM(OutputFormat)

    explicit AudioRecorderEngine(QObject *parent = nullptr);
    ~AudioRecorderEngine() override;

    // Getters
    bool isRecording() const;
    bool isPaused() const;
    bool isPlaying() const;
    qint64 elapsedTime() const;
    QString elapsedTimeFormatted() const;
    int outputFormat() const;
    QString outputDirectory() const;
    int maxDuration() const;
    qint64 playbackPosition() const;
    qint64 playbackDuration() const;
    WaveformProvider* waveform() const;
    RecordingListModel* recordings() const;
    QString lastError() const;

    // Setters
    void setOutputFormat(int format);
    void setOutputDirectory(const QString &dir);
    void setMaxDuration(int seconds);

    // Recording control (callable from QML)
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE void toggleRecording();

    // Playback control (callable from QML)
    Q_INVOKABLE void playRecording(const QString &filePath);
    Q_INVOKABLE void playRecordingAt(int index);
    Q_INVOKABLE void stopPlayback();
    Q_INVOKABLE void seekPlayback(qint64 position);

    // Utility
    Q_INVOKABLE void openOutputDirectory();
    Q_INVOKABLE QString generateFileName() const;

signals:
    void recordingStateChanged();
    void playbackStateChanged();
    void elapsedTimeChanged();
    void outputFormatChanged();
    void outputDirectoryChanged();
    void maxDurationChanged();
    void playbackPositionChanged();
    void playbackDurationChanged();
    void errorOccurred(const QString &message);
    void recordingSaved(const QString &filePath);

private slots:
    void onRecorderStateChanged(QMediaRecorder::RecorderState state);
    void onRecorderError(QMediaRecorder::Error error, const QString &errorString);
    void onPlayerStateChanged(QMediaPlayer::PlaybackState state);
    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);
    void onTimerTick();

private:
    /**
     * @brief Configure the media format based on outputFormat setting.
     */
    void applyMediaFormat();

    /**
     * @brief Generate the full output file path for a new recording.
     */
    QString generateOutputPath() const;

    /**
     * @brief Calculate RMS level from raw audio buffer data.
     */
    void processAudioBuffer();

    // Recording components
    QMediaCaptureSession *m_captureSession;
    QMediaRecorder *m_recorder;
    QAudioInput *m_audioInput;

    // Playback components
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    // Level monitoring components
    QAudioSource *m_monitorSource{nullptr};
    QIODevice *m_levelMonitor{nullptr};

    // Sub-components
    WaveformProvider *m_waveform;
    RecordingListModel *m_recordings;

    // Timer
    QTimer m_timer;
    qint64 m_elapsedMs{0};

    // Settings
    OutputFormat m_outputFormat{FormatWAV};
    QString m_outputDirectory;
    int m_maxDuration{0}; // seconds, 0 = unlimited
    QString m_lastError;

    // Current recording path
    QString m_currentRecordingPath;
};

#endif // AUDIORECORDERENGINE_H
