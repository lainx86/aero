#ifndef RECORDINGINFO_H
#define RECORDINGINFO_H

#include <QString>
#include <QDateTime>

/**
 * @brief Data structure representing a single audio recording.
 *
 * Stores metadata about a recorded audio file including its
 * file path, duration, size, and creation timestamp.
 */
struct RecordingInfo
{
    QString fileName;       // Display name (without path)
    QString filePath;       // Full absolute path to the file
    qint64 duration{0};     // Duration in milliseconds
    qint64 fileSize{0};     // File size in bytes
    QDateTime createdAt;    // Creation timestamp

    /**
     * @brief Format duration as mm:ss string.
     */
    QString formattedDuration() const;

    /**
     * @brief Format file size as human-readable string (KB, MB).
     */
    QString formattedSize() const;
};

#endif // RECORDINGINFO_H
