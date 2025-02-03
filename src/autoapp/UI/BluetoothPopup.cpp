#include <f1x/openauto/autoapp/UI/BluetoothPopup.hpp>
#include <utility>
#include <QtCore/qtimer.h>

namespace f1x::openauto::autoapp::UI {

  /**
   * Handles communication to the Bluetooth Popup Window
   * @param hardwareAddress
   * @param parent
   */
  BluetoothPopup::BluetoothPopup(
      const QString &hardwareAddress,
      QObject *parent) :
      QObject(parent),
      m_bluetoothStatus(BluetoothConnectionStatus::BC_NOT_CONFIGURED),
      m_pairedDeviceCount(0),
      m_connectedDeviceCount(0),
      m_adapterPath(getAdapterPathByAddress(hardwareAddress)),
      m_adapterInterface("org.bluez", m_adapterPath, "org.bluez.Adapter1", QDBusConnection::systemBus(), this) {

    listPairedDevices();

    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceFound", this,
                                         SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceConnected", this,
                                         SLOT(onDeviceConnected(QDBusObjectPath)));
    QDBusConnection::systemBus().connect("org.bluez", m_adapterPath, "org.bluez.Adapter1", "DeviceDisconnected", this,
                                         SLOT(onDeviceDisconnected(QDBusObjectPath)));

  }


  // TODO: Rewrite and Rework and Split between Popup and Monitor
  // Monitor needs to know the status of Bluetooth Connectivity,
  // but we also need to know what devices are connected.

  // List BluetoothAdapters
  // Select Adapter
  // Connect to Paired Device
  // List Paired Devices
  // Remove Paired Device(s)
  // List devices available for Pairing

  /**
   * Private function to disconnect from BluetoothDevice by Path
   * @param devicePath
   * @return
   */
  bool BluetoothPopup::disconnectCurrentDevice() {
    if (m_bluetoothConnectedDevice != nullptr) {
      auto reply = m_adapterInterface.call("DisconnectDevice", m_bluetoothConnectedDevice->path);

      if (reply.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "Successfully disconnected from device: " << m_bluetoothConnectedDevice->path.path();
        m_bluetoothConnectedDevice = {};
        m_bluetoothConnectedDevice = nullptr;
        return true;
      } else {
        qWarning() << "Failed to disconnect from device: " << m_bluetoothConnectedDevice->path.path() << " - "
                   << reply.errorMessage();
        m_bluetoothConnectedDevice = {};
        m_bluetoothConnectedDevice = nullptr;
        return false;
      }
    } else {
      return true;
    }
  }

  /**
   * Private function to connect to Bluetooth Device by Path
   * @param devicePath
   * @return
   */
  bool BluetoothPopup::connectToDevice(const BluetoothDevice& device) {

    auto reply = m_adapterInterface.call("ConnectDevice", device.path);

    if (reply.type() == QDBusMessage::ReplyMessage) {
      qDebug() << "Successfully connected to device: " << device.path.path();
      return true;
    } else {
      qWarning() << "Failed to connect to device: " << device.path.path() << " - " << reply.errorMessage();
      return false;
    }
  }

  bool BluetoothPopup::doConnectToPairedDevice(BluetoothDevice device) {
    // Disconnect from current device if connected
    disconnectCurrentDevice();

    // Connect to selected device
    if (connectToDevice(std::move(device))) {
      return true;
      // Handle successful connection
    } else {
      return false;
      // Handle connection failure
    }
  }

  bool BluetoothPopup::doRemovePair(const BluetoothDevice& device) {
    // Construct the object path for the device

    QDBusReply<void> reply = m_adapterInterface.call("RemoveDevice", device.path);
    if (device.path == m_bluetoothConnectedDevice->path) {
      disconnectCurrentDevice();
    }

    if (reply.isValid()) {
      qDebug() << "Successfully unpaired device: " << device.path.path();
      return true;
    } else {
      qWarning() << "Failed to unpair device: " << reply.error().message();
      return false;
    }
  }

  bool BluetoothPopup::doRemoveAllPairs() {
    disconnectCurrentDevice();

    // Get the list of all devices
    QDBusReply<QVariantMap> reply = m_adapterInterface.call("GetProperties");
    if (!reply.isValid()) {
      qWarning() << "Failed to get adapter properties: " << reply.error().message();
      return false;
    }

    QVariantMap properties = reply.value();
    QVariant devicesVar = properties.value("Devices");
    if (!devicesVar.isValid()) {
      qWarning() << "No devices found.";
      return true; // No devices to remove, so consider this a success
    }

    QList<QDBusObjectPath> devicePaths = devicesVar.value<QList<QDBusObjectPath>>();

    bool allSuccess = true;
    for (const QDBusObjectPath &path : devicePaths) {
      QDBusReply<void> removeReply = m_adapterInterface.call("RemoveDevice", path.path());
      if (!removeReply.isValid()) {
        qWarning() << "Failed to remove device " << path.path() << ": " << removeReply.error().message();
        allSuccess = false;
      } else {
        qDebug() << "Successfully removed device: " << path.path();
      }
    }

    return allSuccess;
  }

  bool BluetoothPopup::doScanDevicesForPairing() {


    // Stop any previous discovery first
    QDBusReply<void> stopReply = m_adapterInterface.call("StopDiscovery");
    if (!stopReply.isValid()) {
      qWarning() << "Failed to stop previous discovery: " << stopReply.error().message();
    }

    m_foundDevices.clear();
    emit discoveredDevicesChanged();

    // Start discovery
    QDBusReply<void> startReply = m_adapterInterface.call("StartDiscovery");
    if (!startReply.isValid()) {
      qWarning() << "Failed to start discovery: " << startReply.error().message();
      return false;
    }

    qDebug() << "Discovery started.";

    // Optionally, set a timer to stop discovery after some time
    QTimer::singleShot(15000, this, [this]() {
      m_adapterInterface.call("StopDiscovery");
      // Here you might want to do something to indicate that discovery is considered 'done'
      // but it's not necessary if you just keep processing devices as they come in
    });

    return true;
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

  BluetoothConnectionStatus::Value BluetoothPopup::getBluetoothStatus() {
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



  void BluetoothPopup::onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties) {

    QString address = properties.value("Address").toString();
    QString name = properties.value("Name").toString();

    BluetoothDevice newDevice{address, name, path};

    if (!std::any_of(m_foundDevices.begin(), m_foundDevices.end(),
                     [&address](const BluetoothDevice& device) { return device.address == address; })) {
      m_foundDevices.append(newDevice);
      emit discoveredDevicesChanged();
      qDebug() << "New device found:" << address << " - " << name << " at path:" << path.path();
    } else {
      qDebug() << "Device already known:" << address;
    }
  }

  BluetoothDevice* BluetoothPopup::findDeviceByPath(const QDBusObjectPath &path) {
    auto it = std::find_if(m_foundDevices.begin(), m_foundDevices.end(),
                           [&path](const BluetoothDevice &device) {
                             return device.path == path;
                           });

    if (it != m_foundDevices.end()) {
      return &(*it);  // Return pointer to the found device
    } else {
      return nullptr;  // Return nullptr if no device found with that path
    }
  }

// Slot for when a device connects
  void BluetoothPopup::onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();

      m_connectedDeviceCount++;
      emit connectedDeviceCountChanged();

      m_bluetoothConnectedDevice = findDeviceByPath(path);
      emit bluetoothConnectedDeviceChanged();

      qDebug() << "Device connected:" << properties.value("Address").toString();
      m_bluetoothStatus = BluetoothConnectionStatus::Value::BC_CONNECTED;
      emit bluetoothStatusChanged();
    }
  }

// Slot for when a device disconnects
  void BluetoothPopup::onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      m_connectedDeviceCount--;
      emit connectedDeviceCountChanged();
      m_bluetoothConnectedDevice = nullptr;
      bluetoothConnectedDeviceChanged();

      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      setBluetoothStatus(BluetoothConnectionStatus::Value::BC_DISCONNECTED);
    }
  }

  int BluetoothPopup::getConnectedDeviceCount() {
    return m_connectedDeviceCount;
  }

  int BluetoothPopup::getPairedDeviceCount() {
    return m_pairedDeviceCount;
  }

  void BluetoothPopup::setBluetoothConnectedDevice(BluetoothDevice* value) {
    if (m_bluetoothConnectedDevice != value) {
      m_bluetoothConnectedDevice = value;
      emit bluetoothConnectedDeviceChanged();
    }
  }

  BluetoothDevice* BluetoothPopup::getBluetoothConnectedDevice() {
    return m_bluetoothConnectedDevice;
  }

  QList<BluetoothDevice> BluetoothPopup::getDiscoveredDevices() const { return m_foundDevices; }
}

