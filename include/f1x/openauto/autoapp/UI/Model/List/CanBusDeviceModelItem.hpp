#ifndef OPENAUTO_CANBUSDEVICEMODELITEM_HPP
#define OPENAUTO_CANBUSDEVICEMODELITEM_HPP

#include <f1x/openauto/autoapp/UI/Model/List/CanBusPortInfoItem.hpp>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace f1x::openauto::autoapp::UI::Model::List {

// Represents a single connected CAN bus peripheral.
// canPorts exposes its physical ports as CanBusPortInfoItem objects for QML.
// status is "ok" (fully configured) or "updating" (awaiting config push).

class CanBusDeviceModelItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString          deviceId        READ getDeviceId        CONSTANT)
    Q_PROPERTY(QString          firmwareVersion READ getFirmwareVersion CONSTANT)
    Q_PROPERTY(QString          status          READ getStatus          NOTIFY statusChanged)
    Q_PROPERTY(QList<QObject*>  canPorts        READ getCanPorts        NOTIFY canPortsChanged)

public:
    CanBusDeviceModelItem(const QString&     deviceId,
                          const QString&     firmwareVersion,
                          const QString&     status,
                          const QVariantList& ports,
                          QObject*           parent = nullptr);

    QString         getDeviceId()        const { return m_deviceId; }
    QString         getFirmwareVersion() const { return m_firmwareVersion; }
    QString         getStatus()          const { return m_status; }
    QList<QObject*> getCanPorts()        const;

    // Called when the device re-announces (e.g. after a config push)
    void updateFromAnnounce(const QString&     firmwareVersion,
                            const QString&     status,
                            const QVariantList& ports);

signals:
    void statusChanged();
    void canPortsChanged();

private:
    void rebuildPorts(const QVariantList& ports);

    QString                    m_deviceId;
    QString                    m_firmwareVersion;
    QString                    m_status;
    QList<CanBusPortInfoItem*> m_canPorts;
};

} // namespace f1x::openauto::autoapp::UI::Model::List

#endif // OPENAUTO_CANBUSDEVICEMODELITEM_HPP