#ifndef RECORDINGLISTMODEL_H
#define RECORDINGLISTMODEL_H

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QList>
#include "RecordingInfo.h"

/**
 * @brief List model providing recording data to QML ListView.
 *
 * Scans a directory for audio files and exposes them with custom roles.
 * Supports delete, rename, and refresh operations.
 * Monitors the directory for external changes via QFileSystemWatcher.
 */
class RecordingListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
    Q_PROPERTY(int playingIndex READ playingIndex WRITE setPlayingIndex NOTIFY playingIndexChanged)

public:
    enum Roles {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        DurationRole,
        FormattedDurationRole,
        FileSizeRole,
        FormattedSizeRole,
        CreatedAtRole,
        IsPlayingRole
    };

    explicit RecordingListModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int count() const;
    QString directory() const;
    void setDirectory(const QString &dir);
    int playingIndex() const;
    void setPlayingIndex(int index);

    // Operations callable from QML
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool deleteRecording(int index);
    Q_INVOKABLE bool renameRecording(int index, const QString &newName);

    /**
     * @brief Update the duration of a recording at the given index.
     * Called after playback resolves the actual duration.
     */
    void updateDuration(int index, qint64 durationMs);

signals:
    void countChanged();
    void directoryChanged();
    void playingIndexChanged();
    void errorOccurred(const QString &message);

private:
    /**
     * @brief Scan the recordings directory and populate the model.
     */
    void scanDirectory();

    /**
     * @brief Probe a file's duration using QMediaPlayer (async).
     */
    void probeFileDuration(int index, const QString &filePath);

    QList<RecordingInfo> m_recordings;
    QString m_directory;
    int m_playingIndex{-1};
    QFileSystemWatcher m_watcher;
};

#endif // RECORDINGLISTMODEL_H
