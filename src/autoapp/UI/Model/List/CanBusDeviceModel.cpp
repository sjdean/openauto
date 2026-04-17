#include <f1x/openauto/autoapp/UI/Model/List/CanBusDeviceModel.hpp>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcCanBusDevice, "journeyos.canbus.device")

namespace f1x::openauto::autoapp::UI::Model::List {

CanBusDeviceModel::CanBusDeviceModel(QObject* parent)
    : QObject(parent)
{}

QList<QObject*> CanBusDeviceModel::getDevices() const
{
    QList<QObject*> list;
    list.reserve(m_devices.size());
    for (auto* item : m_devices)
        list.append(item);
    return list;
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void CanBusDeviceModel::onDeviceAnnounced(const QString&     deviceId,
                                          const QString&     firmwareVersion,
                                          const QString&     status,
                                          const QVariantList& canPorts)
{
    if (auto* existing = findDevice(deviceId)) {
        // Device re-announced (e.g. after config push) — update in place
        qCDebug(lcCanBusDevice) << "re-announced:" << deviceId << status;
        existing->updateFromAnnounce(firmwareVersion, status, canPorts);
        emit devicesChanged();
        return;
    }

    qCDebug(lcCanBusDevice) << "announced:" << deviceId << "fw" << firmwareVersion << status;
    auto* item = new CanBusDeviceModelItem(deviceId, firmwareVersion, status, canPorts, this);
    m_devices.append(item);
    emit devicesChanged();
}

void CanBusDeviceModel::onDeviceDisconnected(const QString& deviceId)
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices.at(i)->getDeviceId() == deviceId) {
            qCDebug(lcCanBusDevice) << "disconnected:" << deviceId;
            m_devices.at(i)->deleteLater();
            m_devices.removeAt(i);
            emit devicesChanged();
            return;
        }
    }
}

// ─── Private ──────────────────────────────────────────────────────────────────

CanBusDeviceModelItem* CanBusDeviceModel::findDevice(const QString& deviceId) const
{
    for (auto* item : m_devices) {
        if (item->getDeviceId() == deviceId)
            return item;
    }
    return nullptr;
}

} // namespace f1x::openauto::autoapp::UI::Model::List