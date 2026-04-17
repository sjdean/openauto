#ifndef OPENAUTO_CANBUSDEVICEMODEL_HPP
#define OPENAUTO_CANBUSDEVICEMODEL_HPP

#include <f1x/openauto/autoapp/UI/Model/List/CanBusDeviceModelItem.hpp>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Model::List {

// Live list of CAN bus peripherals visible to DeviceManager.
//
// Connect DeviceManager's signals to the public slots before exposing this
// object to QML:
//
//   connect(&deviceManager, &DeviceManager::deviceAnnounced,
//           &model,         &CanBusDeviceModel::onDeviceAnnounced);
//   connect(&deviceManager, &DeviceManager::deviceDisconnected,
//           &model,         &CanBusDeviceModel::onDeviceDisconnected);
//
// The QML context property "canBusDeviceModel" then provides:
//   canBusDeviceModel.devices  — QList<QObject*> of CanBusDeviceModelItem

class CanBusDeviceModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> devices READ getDevices NOTIFY devicesChanged)

public:
    explicit CanBusDeviceModel(QObject* parent = nullptr);

    QList<QObject*> getDevices() const;

signals:
    void devicesChanged();

public slots:
    void onDeviceAnnounced(const QString&     deviceId,
                           const QString&     firmwareVersion,
                           const QString&     status,
                           const QVariantList& canPorts);

    void onDeviceDisconnected(const QString& deviceId);

private:
    CanBusDeviceModelItem* findDevice(const QString& deviceId) const;

    QList<CanBusDeviceModelItem*> m_devices;
};

} // namespace f1x::openauto::autoapp::UI::Model::List

#endif // OPENAUTO_CANBUSDEVICEMODEL_HPP