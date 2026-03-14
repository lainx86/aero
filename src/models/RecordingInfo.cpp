#include "RecordingInfo.h"

QString RecordingInfo::formattedDuration() const
{
    int totalSeconds = static_cast<int>(duration / 1000);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

QString RecordingInfo::formattedSize() const
{
    if (fileSize < 1024) {
        return QStringLiteral("%1 B").arg(fileSize);
    } else if (fileSize < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(fileSize / 1024.0, 0, 'f', 1);
    } else {
        return QStringLiteral("%1 MB").arg(fileSize / (1024.0 * 1024.0), 0, 'f', 1);
    }
}
