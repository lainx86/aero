#include "AudioInputDeviceModel.h"

#include <QMediaDevices>
#include <QAudioDevice>

AudioInputDeviceModel::AudioInputDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QMediaDevices *mediaDevices = new QMediaDevices(this);
    connect(mediaDevices, &QMediaDevices::audioInputsChanged,
            this, &AudioInputDeviceModel::updateDevices);

    updateDevices();
}

int AudioInputDeviceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

QVariant AudioInputDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.size())
        return {};

    const QAudioDevice &device = m_devices.at(index.row());
    const QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();

    switch (role) {
    case DeviceNameRole:    return device.description();
    case DeviceIdRole:      return device.id();
    case IsDefaultRole:     return device.id() == defaultDevice.id();
    default:                return {};
    }
}

QHash<int, QByteArray> AudioInputDeviceModel::roleNames() const
{
    return {
        {DeviceNameRole, "deviceName"},
        {DeviceIdRole,   "deviceId"},
        {IsDefaultRole,  "isDefault"}
    };
}

int AudioInputDeviceModel::count() const
{
    return m_devices.size();
}

QAudioDevice AudioInputDeviceModel::deviceAt(int index) const
{
    if (index < 0 || index >= m_devices.size())
        return QAudioDevice();
    return m_devices.at(index);
}

int AudioInputDeviceModel::defaultDeviceIndex() const
{
    const QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices.at(i).id() == defaultDevice.id())
            return i;
    }
    return -1;
}

int AudioInputDeviceModel::indexOf(const QByteArray &id) const
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices.at(i).id() == id)
            return i;
    }
    return -1;
}

void AudioInputDeviceModel::updateDevices()
{
    beginResetModel();
    m_devices = QMediaDevices::audioInputs();
    endResetModel();
    emit countChanged();
}
