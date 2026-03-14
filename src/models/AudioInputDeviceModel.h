#ifndef AUDIOINPUTDEVICEMODEL_H
#define AUDIOINPUTDEVICEMODEL_H

#include <QAbstractListModel>
#include <QAudioDevice>
#include <QList>

/**
 * @brief List model providing available audio input devices to QML ListView.
 *
 * Scans available audio input devices and exposes their name and ID.
 * Monitors for plugging/unplugging of devices via QMediaDevices.
 */
class AudioInputDeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        DeviceNameRole = Qt::UserRole + 1,
        DeviceIdRole,
        IsDefaultRole
    };

    explicit AudioInputDeviceModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int count() const;

    /**
     * @brief Get the QAudioDevice at the specified index.
     */
    QAudioDevice deviceAt(int index) const;

    /**
     * @brief Find the index of the default audio input device.
     */
    Q_INVOKABLE int defaultDeviceIndex() const;

    /**
     * @brief Find the index of a device by its ID.
     */
    Q_INVOKABLE int indexOf(const QByteArray &id) const;

signals:
    void countChanged();

private slots:
    void updateDevices();

private:
    QList<QAudioDevice> m_devices;
};

#endif // AUDIOINPUTDEVICEMODEL_H
