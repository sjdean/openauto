#include <f1x/openauto/autoapp/UI/Model/List/CanBusDeviceModelItem.hpp>
#include <QVariantMap>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcCanBusDeviceItem, "journeyos.canbus.device.item")

namespace f1x::openauto::autoapp::UI::Model::List {

CanBusDeviceModelItem::CanBusDeviceModelItem(const QString&     deviceId,
                                             const QString&     firmwareVersion,
                                             const QString&     status,
                                             const QVariantList& ports,
                                             QObject*           parent)
    : QObject(parent)
    , m_deviceId(deviceId)
    , m_firmwareVersion(firmwareVersion)
    , m_status(status)
{
    rebuildPorts(ports);
}

QList<QObject*> CanBusDeviceModelItem::getCanPorts() const
{
    QList<QObject*> list;
    list.reserve(m_canPorts.size());
    for (auto* item : m_canPorts)
        list.append(item);
    return list;
}

void CanBusDeviceModelItem::updateFromAnnounce(const QString&     firmwareVersion,
                                               const QString&     status,
                                               const QVariantList& ports)
{
    m_firmwareVersion = firmwareVersion;

    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }

    rebuildPorts(ports);
    emit canPortsChanged();
}

void CanBusDeviceModelItem::rebuildPorts(const QVariantList& ports)
{
    qDeleteAll(m_canPorts);
    m_canPorts.clear();
    m_canPorts.reserve(ports.size());

    for (const QVariant& v : ports) {
        const QVariantMap pm = v.toMap();
        auto* item = new CanBusPortInfoItem(
            pm.value(QStringLiteral("portId")).toString(),
            pm.value(QStringLiteral("bitrate")).toUInt(),
            pm.value(QStringLiteral("fdCapable")).toBool(),
            pm.value(QStringLiteral("linkActive")).toBool(),
            this);
        m_canPorts.append(item);
    }

    qCDebug(lcCanBusDeviceItem) << "device" << m_deviceId
                                << "rebuilt" << m_canPorts.size() << "port(s)";
}

} // namespace f1x::openauto::autoapp::UI::Model::List