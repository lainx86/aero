#include "AudioRecorderEngine.h"

#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <cmath>

/**
 * @brief QIODevice subclass that reads audio data for level monitoring.
 *
 * Acts as an audio sink that computes RMS levels from raw PCM samples
 * and forwards them to WaveformProvider.
 */
class AudioLevelMonitor : public QIODevice
{
public:
    explicit AudioLevelMonitor(WaveformProvider *provider, QObject *parent = nullptr)
        : QIODevice(parent)
        , m_provider(provider)
    {
        open(QIODevice::WriteOnly);
    }

protected:
    qint64 readData(char *, qint64) override { return 0; }

    qint64 writeData(const char *data, qint64 len) override
    {
        // Interpret as 16-bit signed PCM samples
        const auto *samples = reinterpret_cast<const qint16 *>(data);
        int sampleCount = static_cast<int>(len / sizeof(qint16));
        if (sampleCount == 0)
            return len;

        double sum = 0.0;
        for (int i = 0; i < sampleCount; ++i) {
            double s = samples[i] / 32768.0;
            sum += s * s;
        }
        double rms = std::sqrt(sum / sampleCount);
        // Scale for better visual representation
        double level = qBound(0.0, rms * 3.0, 1.0);
        m_provider->pushLevel(level);
        return len;
    }

private:
    WaveformProvider *m_provider;
};

AudioRecorderEngine::AudioRecorderEngine(QObject *parent)
    : QObject(parent)
    , m_captureSession(new QMediaCaptureSession(this))
    , m_recorder(new QMediaRecorder(this))
    , m_audioInput(new QAudioInput(this))
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_waveform(new WaveformProvider(this))
    , m_recordings(new RecordingListModel(this))
    , m_inputDevices(new AudioInputDeviceModel(this))
{
    // Initialize input device index and set the device
    m_currentInputDeviceIndex = m_inputDevices->defaultDeviceIndex();
    QAudioDevice device = m_inputDevices->deviceAt(m_currentInputDeviceIndex);
    if (!device.isNull()) {
        m_audioInput->setDevice(device);
    }

    // Setup capture session for recording
    m_captureSession->setAudioInput(m_audioInput);
    m_captureSession->setRecorder(m_recorder);

    // Setup audio level monitor using a separate QAudioSource
    m_levelMonitor = new AudioLevelMonitor(m_waveform, this);

    QAudioFormat monitorFormat;
    monitorFormat.setSampleRate(16000);
    monitorFormat.setChannelCount(1);
    monitorFormat.setSampleFormat(QAudioFormat::Int16);

    auto defaultDevice = device.isNull() ? QMediaDevices::defaultAudioInput() : device;
    m_monitorSource = new QAudioSource(defaultDevice, monitorFormat, this);

    // Setup playback
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(1.0);

    // Connect recorder signals
    connect(m_recorder, &QMediaRecorder::recorderStateChanged,
            this, &AudioRecorderEngine::onRecorderStateChanged);
    connect(m_recorder, &QMediaRecorder::errorOccurred,
            this, &AudioRecorderEngine::onRecorderError);

    // Connect player signals
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &AudioRecorderEngine::onPlayerStateChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &AudioRecorderEngine::onPlayerError);
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &AudioRecorderEngine::playbackPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &AudioRecorderEngine::playbackDurationChanged);

    // Setup recording timer (updates every 100ms for smooth display)
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &AudioRecorderEngine::onTimerTick);

    // Set default output directory
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                         + QStringLiteral("/aero");
    setOutputDirectory(defaultDir);

    // Apply default format
    applyMediaFormat();
}

AudioRecorderEngine::~AudioRecorderEngine()
{
    if (isRecording())
        m_recorder->stop();
    if (isPlaying())
        m_player->stop();
    if (m_monitorSource)
        m_monitorSource->stop();
}

// ======== Getters ========

bool AudioRecorderEngine::isRecording() const
{
    return m_recorder->recorderState() == QMediaRecorder::RecordingState;
}

bool AudioRecorderEngine::isPaused() const
{
    return m_recorder->recorderState() == QMediaRecorder::PausedState;
}

bool AudioRecorderEngine::isPlaying() const
{
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

qint64 AudioRecorderEngine::elapsedTime() const
{
    return m_elapsedMs;
}

QString AudioRecorderEngine::elapsedTimeFormatted() const
{
    int totalSeconds = static_cast<int>(m_elapsedMs / 1000);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

int AudioRecorderEngine::outputFormat() const
{
    return static_cast<int>(m_outputFormat);
}

QString AudioRecorderEngine::outputDirectory() const
{
    return m_outputDirectory;
}

int AudioRecorderEngine::maxDuration() const
{
    return m_maxDuration;
}

qint64 AudioRecorderEngine::playbackPosition() const
{
    return m_player->position();
}

qint64 AudioRecorderEngine::playbackDuration() const
{
    return m_player->duration();
}

WaveformProvider* AudioRecorderEngine::waveform() const
{
    return m_waveform;
}

RecordingListModel* AudioRecorderEngine::recordings() const
{
    return m_recordings;
}

AudioInputDeviceModel* AudioRecorderEngine::inputDevices() const
{
    return m_inputDevices;
}

int AudioRecorderEngine::currentInputDeviceIndex() const
{
    return m_currentInputDeviceIndex;
}

QString AudioRecorderEngine::lastError() const
{
    return m_lastError;
}

// ======== Setters ========

void AudioRecorderEngine::setOutputFormat(int format)
{
    auto f = static_cast<OutputFormat>(qBound(0, format, 2));
    if (m_outputFormat == f)
        return;

    m_outputFormat = f;
    applyMediaFormat();
    emit outputFormatChanged();
}

void AudioRecorderEngine::setOutputDirectory(const QString &dir)
{
    if (m_outputDirectory == dir)
        return;

    m_outputDirectory = dir;

    // Ensure directory exists
    QDir d(m_outputDirectory);
    if (!d.exists())
        d.mkpath(QStringLiteral("."));

    m_recordings->setDirectory(m_outputDirectory);
    emit outputDirectoryChanged();
}

void AudioRecorderEngine::setMaxDuration(int seconds)
{
    if (m_maxDuration == seconds)
        return;

    m_maxDuration = qMax(0, seconds);
    emit maxDurationChanged();
}

void AudioRecorderEngine::setCurrentInputDeviceIndex(int index)
{
    if (m_currentInputDeviceIndex == index || index < 0 || index >= m_inputDevices->count())
        return;

    m_currentInputDeviceIndex = index;
    QAudioDevice device = m_inputDevices->deviceAt(index);

    // Stop current recording/monitoring if active
    bool wasRecording = isRecording();
    bool wasPaused = isPaused();
    if (wasRecording || wasPaused) {
        m_recorder->stop();
        m_monitorSource->stop();
        m_timer.stop();
    }

    // Update the audio input device for QMediaRecorder
    m_audioInput->setDevice(device);

    // Recreate monitor source for the new device
    if (m_monitorSource) {
        delete m_monitorSource;
    }
    QAudioFormat monitorFormat;
    monitorFormat.setSampleRate(16000);
    monitorFormat.setChannelCount(1);
    monitorFormat.setSampleFormat(QAudioFormat::Int16);
    m_monitorSource = new QAudioSource(device, monitorFormat, this);

    emit currentInputDeviceIndexChanged();

    // Resume recording if it was active
    if (wasRecording) {
        m_monitorSource->start(m_levelMonitor);
        m_recorder->record();
        m_timer.start();
    } else if (wasPaused) {
        // Technically can't seamlessly resume a paused recorder after device change
        // but we'll try to leave it stopped or emit a state change appropriately.
    }
}

// ======== Recording Control ========

void AudioRecorderEngine::startRecording()
{
    if (isRecording())
        return;

    // Stop playback if active
    if (isPlaying())
        stopPlayback();

    // Generate output path
    m_currentRecordingPath = generateOutputPath();
    m_recorder->setOutputLocation(QUrl::fromLocalFile(m_currentRecordingPath));

    // Reset timer and waveform
    m_elapsedMs = 0;
    emit elapsedTimeChanged();
    m_waveform->reset();
    m_waveform->setActive(true);

    // Start the audio level monitor
    m_monitorSource->start(m_levelMonitor);

    // Start recording
    m_recorder->record();
    m_timer.start();
}

void AudioRecorderEngine::stopRecording()
{
    if (!isRecording() && !isPaused())
        return;

    m_recorder->stop();
    m_timer.stop();

    // Stop the level monitor
    m_monitorSource->stop();

    m_waveform->setActive(false);

    // Refresh recordings list to show the new file
    m_recordings->refresh();

    emit recordingSaved(m_currentRecordingPath);
}

void AudioRecorderEngine::pauseRecording()
{
    if (!isRecording())
        return;

    m_recorder->pause();
    m_timer.stop();
    m_monitorSource->stop();
}

void AudioRecorderEngine::resumeRecording()
{
    if (!isPaused())
        return;

    m_monitorSource->start(m_levelMonitor);
    m_recorder->record();
    m_timer.start();
}

void AudioRecorderEngine::toggleRecording()
{
    if (isRecording()) {
        stopRecording();
    } else if (isPaused()) {
        resumeRecording();
    } else {
        startRecording();
    }
}

// ======== Playback Control ========

void AudioRecorderEngine::playRecording(const QString &filePath)
{
    // Stop current recording if active
    if (isRecording())
        stopRecording();

    m_player->setSource(QUrl::fromLocalFile(filePath));
    m_player->play();
}

void AudioRecorderEngine::playRecordingAt(int index)
{
    if (index < 0 || index >= m_recordings->count())
        return;

    auto modelIndex = m_recordings->index(index, 0);
    QString path = m_recordings->data(modelIndex, RecordingListModel::FilePathRole).toString();

    m_recordings->setPlayingIndex(index);
    playRecording(path);
}

void AudioRecorderEngine::stopPlayback()
{
    m_player->stop();
    m_recordings->setPlayingIndex(-1);
}

void AudioRecorderEngine::seekPlayback(qint64 position)
{
    m_player->setPosition(position);
}

// ======== Utility ========

void AudioRecorderEngine::openOutputDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_outputDirectory));
}

QString AudioRecorderEngine::generateFileName() const
{
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_hh-mm-ss"));

    QString ext;
    switch (m_outputFormat) {
    case FormatWAV: ext = QStringLiteral("wav"); break;
    case FormatMP3: ext = QStringLiteral("mp3"); break;
    case FormatOGG: ext = QStringLiteral("ogg"); break;
    }

    return QStringLiteral("aero_%1.%2").arg(timestamp, ext);
}

// ======== Private Slots ========

void AudioRecorderEngine::onRecorderStateChanged(QMediaRecorder::RecorderState state)
{
    Q_UNUSED(state)
    emit recordingStateChanged();
}

void AudioRecorderEngine::onRecorderError(QMediaRecorder::Error error, const QString &errorString)
{
    Q_UNUSED(error)
    m_lastError = errorString;
    qWarning() << "[AudioRecorderEngine] Recorder error:" << errorString;
    emit errorOccurred(errorString);
}

void AudioRecorderEngine::onPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState) {
        // Update duration for the recording that just finished
        qint64 dur = m_player->duration();
        int idx = m_recordings->playingIndex();
        if (dur > 0 && idx >= 0) {
            m_recordings->updateDuration(idx, dur);
        }
        m_recordings->setPlayingIndex(-1);
    }
    emit playbackStateChanged();
}

void AudioRecorderEngine::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error)
    m_lastError = errorString;
    qWarning() << "[AudioRecorderEngine] Player error:" << errorString;
    emit errorOccurred(errorString);
}

void AudioRecorderEngine::onTimerTick()
{
    m_elapsedMs += m_timer.interval();
    emit elapsedTimeChanged();

    // Auto-stop if max duration is set
    if (m_maxDuration > 0 && m_elapsedMs >= m_maxDuration * 1000) {
        qDebug() << "[AudioRecorderEngine] Max duration reached, stopping recording.";
        stopRecording();
    }
}

// ======== Private Methods ========

void AudioRecorderEngine::applyMediaFormat()
{
    QMediaFormat format;

    switch (m_outputFormat) {
    case FormatWAV:
        format.setFileFormat(QMediaFormat::Wave);
        format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
        break;
    case FormatMP3:
        format.setFileFormat(QMediaFormat::MP3);
        format.setAudioCodec(QMediaFormat::AudioCodec::MP3);
        break;
    case FormatOGG:
        format.setFileFormat(QMediaFormat::Ogg);
        format.setAudioCodec(QMediaFormat::AudioCodec::Vorbis);
        break;
    }

    m_recorder->setMediaFormat(format);
    m_recorder->setQuality(QMediaRecorder::HighQuality);
}

QString AudioRecorderEngine::generateOutputPath() const
{
    return m_outputDirectory + QStringLiteral("/") + generateFileName();
}
