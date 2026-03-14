#ifndef WAVEFORMPROVIDER_H
#define WAVEFORMPROVIDER_H

#include <QObject>
#include <QList>
#include <QTimer>

/**
 * @brief Provides real-time audio level data for waveform visualization.
 *
 * Maintains a rolling buffer of audio level samples and exposes them
 * to QML for rendering. Also tracks the current peak level for VU meter.
 */
class WaveformProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<qreal> samples READ samples NOTIFY samplesChanged)
    Q_PROPERTY(qreal currentLevel READ currentLevel NOTIFY currentLevelChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit WaveformProvider(QObject *parent = nullptr);

    QList<qreal> samples() const;
    qreal currentLevel() const;
    bool isActive() const;
    void setActive(bool active);

    /**
     * @brief Push a new audio level sample (0.0 to 1.0).
     * Called by AudioRecorderEngine when a new audio buffer is processed.
     */
    void pushLevel(qreal level);

    /**
     * @brief Reset all samples to zero.
     */
    Q_INVOKABLE void reset();

signals:
    void samplesChanged();
    void currentLevelChanged();
    void activeChanged();

private:
    static constexpr int kMaxSamples = 128; // Number of bars in waveform

    QList<qreal> m_samples;
    qreal m_currentLevel{0.0};
    bool m_active{false};
    QTimer m_decayTimer;
};

#endif // WAVEFORMPROVIDER_H
