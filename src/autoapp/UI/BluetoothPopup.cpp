#include <f1x/openauto/autoapp/UI/BluetoothPopup.hpp>

namespace f1x::openauto::autoapp::UI {
// TODO: Rename BluetoothMonitor
// TODO: Add additional PIN activity
  BluetoothPopup::BluetoothPopup(
      const QString &hardwareAddress,
      QObject *parent) :
      m_bluetoothStatus(BluetoothConnectionStatus::BC_NOT_CONFIGURED),
      m_pairedDeviceCount(0),
      m_connectedDeviceCount(0),
      m_adapterPath(getAdapterPathByAddress(hardwareAddress)),
      m_adapterInterface("org.bluez", m_adapterPath, "org.bluez.Adapter1", QDBusConnection::systemBus(), this),
      QObject(parent) {

    listPairedDevices();

    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceFound", this,
                                         SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceConnected", this,
                                         SLOT(onDeviceConnected(QDBusObjectPath)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceDisconnected", this,
                                         SLOT(onDeviceDisconnected(QDBusObjectPath)));

  }

  QString BluetoothPopup::getAdapterPathByAddress(const QString &adapterAddress) {
    QDBusInterface manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusReply<QVariantMap> reply = manager.call("GetManagedObjects");

    if (!reply.isValid()) {
      qWarning() << "Error getting managed objects:" << reply.error().message();
      return QString();
    }

    QVariantMap managedObjects = reply.value();
    for (auto it = managedObjects.begin(); it != managedObjects.end(); ++it) {
      QVariantMap interfaces = it->toMap();
      if (interfaces.contains("org.bluez.Adapter1")) {
        QVariantMap adapterProperties = interfaces["org.bluez.Adapter1"].toMap();
        QString address = adapterProperties["Address"].toString();
        if (address == adapterAddress) {
          return it.key();
        }
      }
    }
    return {}; // No matching adapter found
  }

  void BluetoothPopup::setBluetoothStatus(BluetoothConnectionStatus value) {
    if (m_bluetoothStatus != value) {
      m_bluetoothStatus = value;
      emit bluetoothStatusChanged();
    }
  }

  BluetoothConnectionStatus BluetoothPopup::getBluetoothStatus() {
    return m_bluetoothStatus;
  }

  void BluetoothPopup::setBluetoothLocalDevice(QString value) {
    if (m_bluetoothLocalDevice != value) {
      m_bluetoothLocalDevice = value;
      emit bluetoothLocalDeviceChanged();
    }
  }

  QString BluetoothPopup::getBluetoothLocalDevice() {
    // TODO: This needs to be set from the Name/Address of the Adapter
    return m_bluetoothLocalDevice;
  }

// Helper function to convert QDBusArgument to QVariantMap
  static QVariantMap dbusArgumentToVariantMap(const QDBusArgument &arg) {
    QVariantMap map;
    arg.beginMap();
    while (!arg.atEnd()) {
      QString key;
      QVariant value;
      arg.beginMapEntry();
      arg >> key >> value;
      arg.endMapEntry();
      map.insert(key, value);
    }
    arg.endMap();
    return map;
  }

  void BluetoothPopup::listPairedDevices() {
    auto reply = m_adapterInterface.call("GetProperties");
    if (reply.type() == QDBusMessage::ReplyMessage) {
      QVariantMap properties = dbusArgumentToVariantMap(reply.arguments().at(0).value<QDBusArgument>());
      QVariant devices = properties.value("Devices");
      if (devices.isValid()) {
        QList<QDBusObjectPath> devicePaths = devices.value<QList<QDBusObjectPath>>();
        for (const QDBusObjectPath &path: devicePaths) {
          QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus());
          QDBusReply<QVariantMap> deviceProperties = device.call("GetProperties");
          if (deviceProperties.isValid()) {
            QVariantMap prop = deviceProperties.value();
            if (prop.value("Paired").toBool()) {
              m_pairedDeviceCount++;
              setBluetoothStatus(BluetoothConnectionStatus::BC_DISCONNECTED);
              qDebug() << "Paired device:" << prop.value("Alias").toString() << "with address:"
                       << prop.value("Address").toString();
            }
          }
        }
      }
    }
  }

  void BluetoothPopup::onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties) {
    qDebug() << "Device found:" << properties.value("Address").toString();
    setBluetoothStatus(BluetoothConnectionStatus::BC_CONNECTING);
  }

// Slot for when a device connects
  void BluetoothPopup::onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();

      m_connectedDeviceCount++;
      emit connectedDeviceCountChanged();

      // TODO: Output Device Info
      m_bluetoothConnectedDevice = "";
      emit bluetoothConnectedDeviceChanged();

      qDebug() << "Device connected:" << properties.value("Address").toString();
      setBluetoothStatus(BluetoothConnectionStatus::BC_CONNECTED);
    }
  }

// Slot for when a device disconnects
  void BluetoothPopup::onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      m_connectedDeviceCount--;
      emit connectedDeviceCountChanged();

      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      setBluetoothStatus(BluetoothConnectionStatus::BC_DISCONNECTED);
    }
  }

  int BluetoothPopup::getConnectedDeviceCount() {
    return m_connectedDeviceCount;
  }

  int BluetoothPopup::getPairedDeviceCount() {
    return m_pairedDeviceCount;
  }

  void BluetoothPopup::setBluetoothConnectedDevice(QString value) {
    if (m_bluetoothConnectedDevice != value) {
      m_bluetoothConnectedDevice = value;
      emit bluetoothConnectedDeviceChanged();
    }
  }

  QString BluetoothPopup::getBluetoothConnectedDevice() {
    return m_bluetoothConnectedDevice;
  }
}

