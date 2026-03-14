#include "WaveformProvider.h"

WaveformProvider::WaveformProvider(QObject *parent)
    : QObject(parent)
    , m_samples(kMaxSamples, 0.0)
{
    // Decay timer: gradually reduce level when no new data arrives
    m_decayTimer.setInterval(50);
    connect(&m_decayTimer, &QTimer::timeout, this, [this]() {
        if (m_currentLevel > 0.001) {
            m_currentLevel *= 0.85;
            emit currentLevelChanged();
        } else if (m_currentLevel > 0.0) {
            m_currentLevel = 0.0;
            emit currentLevelChanged();
        }
    });
}

QList<qreal> WaveformProvider::samples() const
{
    return m_samples;
}

qreal WaveformProvider::currentLevel() const
{
    return m_currentLevel;
}

bool WaveformProvider::isActive() const
{
    return m_active;
}

void WaveformProvider::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    emit activeChanged();

    if (m_active) {
        m_decayTimer.start();
    } else {
        m_decayTimer.stop();
        // Gradual fade-out is handled by the decay timer
    }
}

void WaveformProvider::pushLevel(qreal level)
{
    // Clamp to [0, 1]
    level = qBound(0.0, level, 1.0);

    // Update current level
    m_currentLevel = level;
    emit currentLevelChanged();

    // Shift samples left and append new value
    if (m_samples.size() >= kMaxSamples) {
        m_samples.removeFirst();
    }
    m_samples.append(level);
    emit samplesChanged();
}

void WaveformProvider::reset()
{
    m_samples.fill(0.0, kMaxSamples);
    m_currentLevel = 0.0;
    emit samplesChanged();
    emit currentLevelChanged();
}
