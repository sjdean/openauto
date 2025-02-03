#include <f1x/openauto/autoapp/UI/BluetoothMonitor.hpp>

namespace f1x::openauto::autoapp::UI {

  BluetoothMonitor::BluetoothMonitor(
      const QString &hardwareAddress,
      QObject *parent) :
      QObject(parent),
      m_bluetoothLocalDevice(), // TODO Bring in Local Address from Configuration
      m_bluetoothStatus(BluetoothConnectionStatus::BC_NOT_CONFIGURED),
      m_pairedDeviceCount(0),
      m_connectedDeviceCount(0),
      m_adapterPath(getAdapterPathByAddress(hardwareAddress)),
      m_adapterInterface("org.bluez", m_adapterPath, "org.bluez.Adapter1", QDBusConnection::systemBus(), this) {

    initialiseMonitor();

    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceFound", this,
                                         SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceConnected", this,
                                         SLOT(onDeviceConnected(QDBusObjectPath)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceDisconnected", this,
                                         SLOT(onDeviceDisconnected(QDBusObjectPath)));

  }

  /**
   * Private function to get the BlueZ adapter interface by hardware address
   * @param adapterAddress
   * @return BlueZ interface
   */
  QString BluetoothMonitor::getAdapterPathByAddress(const QString &adapterAddress) {
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

  /**
   * Provides an indication of the Bluetooth Connectivity Status
   * @return
   */
  BluetoothConnectionStatus::Value BluetoothMonitor::getBluetoothStatus() {
    return m_bluetoothStatus;
  }

  /**
   * Set the Adapter Device by hardware address
   * @param value
   */
  void BluetoothMonitor::setBluetoothLocalDevice(QString value) {
    if (m_bluetoothLocalDevice != value) {
      // TODO: Reconfigure BlueZ with adapter
      // TODO: Update Configuation
      m_bluetoothLocalDevice = value;
      emit bluetoothLocalDeviceChanged();
    }
  }

  /**
   * Return the hardware address of the configured Bluetooth Adapter
   * @return
   */
  QString BluetoothMonitor::getBluetoothLocalDevice() {
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

  /**
   * Private function to initialise the Monitor
   */
  void BluetoothMonitor::initialiseMonitor() {
    m_devices.clear();
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

              std::string alias = prop.value("Alias").toString().toStdString();
              std::string address = prop.value("Address").toString().toStdString();
              m_devices.emplace_back(alias, address);

              m_bluetoothStatus = BluetoothConnectionStatus::Value::BC_DISCONNECTED;
              emit bluetoothStatusChanged();

              qDebug() << "Paired device:" << prop.value("Alias").toString() << "with address:"
                       << prop.value("Address").toString();
            }
          }
        }
      }
    }
  }

  std::vector<std::pair<std::string, std::string>> BluetoothMonitor::getDeviceList() {
    return m_devices;
  }

  /**
   * When device is found with BlueZ
   * @param path
   * @param properties
   */
  void BluetoothMonitor::onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties) {
    qDebug() << "Device found:" << properties.value("Address").toString();
    m_bluetoothStatus = BluetoothConnectionStatus::Value::BC_CONNECTING;
    emit bluetoothStatusChanged();
  }

  /**
   * When device disconnects on BlueZ
   * @param path
   */
  void BluetoothMonitor::onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();

      m_connectedDeviceCount++;
      emit connectedDeviceCountChanged();

      m_bluetoothConnectedDevice = properties.value("Alias").toString();
      // TODO: Add the Address in Too!
      emit bluetoothConnectedDeviceChanged();

      m_bluetoothStatus = BluetoothConnectionStatus::Value::BC_CONNECTED;
      emit bluetoothStatusChanged();
    }
  }

  /**
   * When Bluetooth Device Connects with BlueZ
   * @param path
   */
  void BluetoothMonitor::onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      m_connectedDeviceCount--;
      emit connectedDeviceCountChanged();

      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      m_bluetoothStatus = BluetoothConnectionStatus::Value::BC_DISCONNECTED;
      emit bluetoothStatusChanged();
    }
  }

  int BluetoothMonitor::getConnectedDeviceCount() {
    return m_connectedDeviceCount;
  }

  int BluetoothMonitor::getPairedDeviceCount() {
    return m_pairedDeviceCount;
  }

  void BluetoothMonitor::setBluetoothConnectedDevice(QString value) {
    if (m_bluetoothConnectedDevice != value) {
      m_bluetoothConnectedDevice = value;
      emit bluetoothConnectedDeviceChanged();
    }
  }

  QString BluetoothMonitor::getBluetoothConnectedDevice() {
    return m_bluetoothConnectedDevice;
  }
}

