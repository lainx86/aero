#include "RecordingListModel.h"

#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QUrl>
#include <algorithm>

// Supported audio file extensions
static const QStringList kAudioExtensions = {
    QStringLiteral("wav"), QStringLiteral("mp3"), QStringLiteral("ogg"),
    QStringLiteral("m4a"), QStringLiteral("flac"), QStringLiteral("aac")
};

RecordingListModel::RecordingListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &RecordingListModel::refresh);
}

int RecordingListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_recordings.size());
}

QVariant RecordingListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_recordings.size())
        return {};

    const auto &rec = m_recordings.at(index.row());

    switch (role) {
    case FileNameRole:          return rec.fileName;
    case FilePathRole:          return rec.filePath;
    case DurationRole:          return rec.duration;
    case FormattedDurationRole: return rec.formattedDuration();
    case FileSizeRole:          return rec.fileSize;
    case FormattedSizeRole:     return rec.formattedSize();
    case CreatedAtRole:         return rec.createdAt.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
    case IsPlayingRole:         return index.row() == m_playingIndex;
    default:                    return {};
    }
}

QHash<int, QByteArray> RecordingListModel::roleNames() const
{
    return {
        {FileNameRole,          "fileName"},
        {FilePathRole,          "filePath"},
        {DurationRole,          "duration"},
        {FormattedDurationRole, "formattedDuration"},
        {FileSizeRole,          "fileSize"},
        {FormattedSizeRole,     "formattedSize"},
        {CreatedAtRole,         "createdAt"},
        {IsPlayingRole,         "isPlaying"}
    };
}

int RecordingListModel::count() const
{
    return static_cast<int>(m_recordings.size());
}

QString RecordingListModel::directory() const
{
    return m_directory;
}

void RecordingListModel::setDirectory(const QString &dir)
{
    if (m_directory == dir)
        return;

    // Stop watching old directory
    if (!m_directory.isEmpty())
        m_watcher.removePath(m_directory);

    m_directory = dir;

    // Create directory if it doesn't exist
    QDir d(m_directory);
    if (!d.exists())
        d.mkpath(QStringLiteral("."));

    // Start watching new directory
    m_watcher.addPath(m_directory);

    emit directoryChanged();
    scanDirectory();
}

int RecordingListModel::playingIndex() const
{
    return m_playingIndex;
}

void RecordingListModel::setPlayingIndex(int index)
{
    if (m_playingIndex == index)
        return;

    int oldIndex = m_playingIndex;
    m_playingIndex = index;

    // Update the old and new playing items
    if (oldIndex >= 0 && oldIndex < m_recordings.size()) {
        auto idx = createIndex(oldIndex, 0);
        emit dataChanged(idx, idx, {IsPlayingRole});
    }
    if (m_playingIndex >= 0 && m_playingIndex < m_recordings.size()) {
        auto idx = createIndex(m_playingIndex, 0);
        emit dataChanged(idx, idx, {IsPlayingRole});
    }

    emit playingIndexChanged();
}

void RecordingListModel::refresh()
{
    scanDirectory();
}

bool RecordingListModel::deleteRecording(int index)
{
    if (index < 0 || index >= m_recordings.size())
        return false;

    const QString path = m_recordings.at(index).filePath;
    QFile file(path);

    if (!file.remove()) {
        emit errorOccurred(QStringLiteral("Failed to delete: %1").arg(file.errorString()));
        return false;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_recordings.removeAt(index);
    endRemoveRows();

    // Adjust playing index if needed
    if (m_playingIndex == index) {
        setPlayingIndex(-1);
    } else if (m_playingIndex > index) {
        m_playingIndex--;
        emit playingIndexChanged();
    }

    emit countChanged();
    return true;
}

bool RecordingListModel::renameRecording(int index, const QString &newName)
{
    if (index < 0 || index >= m_recordings.size() || newName.isEmpty())
        return false;

    auto &rec = m_recordings[index];
    QFileInfo fi(rec.filePath);
    QString newPath = fi.absolutePath() + QStringLiteral("/") + newName;

    // Preserve extension if not provided
    if (!newName.contains(QLatin1Char('.')))
        newPath += QStringLiteral(".") + fi.suffix();

    QFile file(rec.filePath);
    if (!file.rename(newPath)) {
        emit errorOccurred(QStringLiteral("Failed to rename: %1").arg(file.errorString()));
        return false;
    }

    QFileInfo newFi(newPath);
    rec.fileName = newFi.fileName();
    rec.filePath = newFi.absoluteFilePath();

    auto idx = createIndex(index, 0);
    emit dataChanged(idx, idx, {FileNameRole, FilePathRole});
    return true;
}

void RecordingListModel::updateDuration(int index, qint64 durationMs)
{
    if (index < 0 || index >= m_recordings.size())
        return;

    m_recordings[index].duration = durationMs;
    auto idx = createIndex(index, 0);
    emit dataChanged(idx, idx, {DurationRole, FormattedDurationRole});
}

void RecordingListModel::scanDirectory()
{
    if (m_directory.isEmpty())
        return;

    beginResetModel();
    m_recordings.clear();

    QDir dir(m_directory);
    QStringList filters;
    for (const auto &ext : kAudioExtensions)
        filters << QStringLiteral("*.%1").arg(ext);

    auto entries = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    for (const auto &fi : entries) {
        RecordingInfo rec;
        rec.fileName = fi.fileName();
        rec.filePath = fi.absoluteFilePath();
        rec.fileSize = fi.size();
        rec.createdAt = fi.birthTime().isValid() ? fi.birthTime() : fi.lastModified();
        rec.duration = 0; // Will be probed asynchronously
        m_recordings.append(rec);
    }

    endResetModel();
    emit countChanged();

    // Probe durations asynchronously for each recording
    for (int i = 0; i < m_recordings.size(); ++i) {
        probeFileDuration(i, m_recordings.at(i).filePath);
    }
}

void RecordingListModel::probeFileDuration(int index, const QString &filePath)
{
    // Create a temporary player to probe duration
    auto *probe = new QMediaPlayer(this);
    probe->setSource(QUrl::fromLocalFile(filePath));

    connect(probe, &QMediaPlayer::durationChanged, this,
            [this, probe, index](qint64 dur) {
                if (dur > 0) {
                    updateDuration(index, dur);
                }
                probe->deleteLater();
            });

    // Clean up on error
    connect(probe, &QMediaPlayer::errorOccurred, this,
            [probe](QMediaPlayer::Error, const QString &) {
                probe->deleteLater();
            });
}
