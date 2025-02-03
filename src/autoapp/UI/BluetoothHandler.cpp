#include <f1x/openauto/autoapp/UI/BluetoothHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  BluetoothHandler::BluetoothHandler(QString hardwareAddress) :
      m_manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus()) {

    // Get Managed Objects
    QDBusReply<QVariantMap> reply = m_manager.call("GetManagedObjects");

    if (reply.isValid()) {
      QVariantMap managedObjects = reply.value();

      m_adapters.clear();
      m_devices.clear();

      // For each device...
      for (auto it = managedObjects.begin(); it != managedObjects.end(); ++it) {

        QVariantMap interfaces = it->toMap();

        // If Adapter...
        if (interfaces.contains("org.bluez.Adapter1")) {
          // Handle existing adapter
          QVariantMap adapterProps = interfaces["org.bluez.Adapter1"].toMap();
          BluetoothAdapter adapter(adapterProps["Address"].toString(),adapterProps["Name"].toString(),it.key(),adapterProps["Powered"].toBool(),adapterProps["Discoverable"].toBool(),adapterProps["Discovering"].toBool());
          m_adapters.append(adapter);
        }

        // If Device...
        if (interfaces.contains("org.bluez.Device1")) {
          QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();

          BluetoothDevice device(deviceProps["Address"].toString(), deviceProps["Name"].toString(), QDBusObjectPath(deviceProps["Path"].toString()), deviceProps["Paired"].toBool(), deviceProps["Connected"].toBool());
          m_devices.append(device);
        }
      }
      emit bluetoothAdapterListChanged();
    } else {
      qWarning() << "Failed to get managed objects:" << reply.error().message();
    }

    // Find Adapter by Hardware Address
    auto it = std::find_if(m_adapters.begin(), m_adapters.end(),
                           [&hardwareAddress](const BluetoothAdapter &adapter) {
                             return adapter.hardwareAddress == hardwareAddress;
                           });

    if (it != m_adapters.end()) {
      m_activeAdapterIndex = std::distance(m_adapters.begin(), it);
      QDBusConnection::systemBus().connect("org.bluez", it->path, "org.bluez.Adapter1", "DeviceFound", this,
                                           SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
      QDBusConnection::systemBus().connect("org.bluez", it->path, "org.bluez.Adapter1", "DeviceConnected", this,
                                           SLOT(onDeviceConnected(QDBusObjectPath)));
      QDBusConnection::systemBus().connect("org.bluez", it->path, "org.bluez.Adapter1", "DeviceDisconnected", this,
                                           SLOT(onDeviceDisconnected(QDBusObjectPath)));
    } else {
      qDebug("Unable to find adapter");
    }
  }

  /* Action Functions */

  /**
 * Private function to disconnect from BluetoothDevice by Path
 * @param devicePath
 * @return
 */
  bool BluetoothHandler::disconnectCurrentDevice() {
    if (m_activeDeviceIndex > -1) {
      auto reply = m_manager.call("DisconnectDevice", m_devices[m_activeDeviceIndex].path);

      if (reply.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "Successfully disconnected from device: " << m_devices[m_activeDeviceIndex].address;
        return true;
      } else {
        qWarning() << "Failed to disconnect from device: " << m_devices[m_activeDeviceIndex].address << " - "
                   << reply.errorMessage();
        m_activeDeviceIndex = -1;
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
  bool BluetoothHandler::connectToDevice(const BluetoothDevice& device) {

    auto reply = m_manager.call("ConnectDevice", m_devices[m_activeDeviceIndex].path);

    if (reply.type() == QDBusMessage::ReplyMessage) {
      qDebug() << "Successfully connected to device: " << m_devices[m_activeDeviceIndex].address;
      return true;
    } else {
      qWarning() << "Failed to connect to device: " << m_devices[m_activeDeviceIndex].address << " - " << reply.errorMessage();
      return false;
    }
  }

  bool BluetoothHandler::doConnectToPairedDevice(BluetoothDevice device) {
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

  bool BluetoothHandler::doRemovePair(const BluetoothDevice& device) {
    // Construct the object path for the device

    QDBusReply<void> reply = m_manager.call("RemoveDevice", device.path);
    if (device.path == m_devices[m_activeDeviceIndex].path) {
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

  bool BluetoothHandler::doRemoveAllPairs() {
    disconnectCurrentDevice();

    // Get the list of all devices
    QDBusReply<QVariantMap> reply = m_manager.call("GetProperties");
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
      QDBusReply<void> removeReply = m_manager.call("RemoveDevice", path.path());
      if (!removeReply.isValid()) {
        qWarning() << "Failed to remove device " << path.path() << ": " << removeReply.error().message();
        allSuccess = false;
      } else {
        qDebug() << "Successfully removed device: " << path.path();
      }
    }

    return allSuccess;
  }

  bool BluetoothHandler::doScanDevicesForPairing() {


    // Stop any previous discovery first
    QDBusReply<void> stopReply = m_manager.call("StopDiscovery");
    if (!stopReply.isValid()) {
      qWarning() << "Failed to stop previous discovery: " << stopReply.error().message();
    }

    m_devices.erase(
        std::remove_if(m_devices.begin(), m_devices.end(),
                       [](const BluetoothDevice &device) { return !device.paired; }),
        m_devices.end()
    );
    emit unpairedDeviceListChanged();

    // Start discovery
    QDBusReply<void> startReply = m_manager.call("StartDiscovery");
    if (!startReply.isValid()) {
      qWarning() << "Failed to start discovery: " << startReply.error().message();
      return false;
    }

    qDebug() << "Discovery started.";

    // Optionally, set a timer to stop discovery after some time
    QTimer::singleShot(15000, this, [this]() {
      m_manager.call("StopDiscovery");
      // Here you might want to do something to indicate that discovery is considered 'done'
      // but it's not necessary if you just keep processing devices as they come in
    });

    return true;
  }

  /* Private Functions */
  void BluetoothHandler::onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties) {
    if (interfacesAndProperties.contains("org.bluez.Device1")) {
      QVariantMap deviceProps = interfacesAndProperties["org.bluez.Device1"].toMap();
      QString address = deviceProps.value("Address").toString();
      QString name = deviceProps.value("Name").toString();

      // Check if this is a new device or an update to an existing one
      auto it = std::find_if(m_devices.begin(), m_devices.end(),
                             [&address](const BluetoothDevice& device) { return device.address == address; });

      if (it == m_devices.end()) {
        // New device
        BluetoothDevice device(deviceProps["Address"].toString(), deviceProps["Name"].toString(),
                               QDBusObjectPath(deviceProps["Path"].toString()),
                               deviceProps["Paired"].toBool(),
                               deviceProps["Connected"].toBool());

        m_devices.append(device);
        if (device.paired) {
          emit pairedDeviceListChanged();
        } else {
          emit unpairedDeviceListChanged();
        }

        qDebug() << "New device found:" << address << " - " << name << " at path:" << objectPath.path();
      } else {

        // Update existing device
        BluetoothDevice &existingDevice = *it;
        if (existingDevice.name != name) {
          existingDevice.name = name;
          qDebug() << "Device name updated for:" << address << " to " << name;
        }
        if (existingDevice.paired != deviceProps["Paired"].toBool()) {
          existingDevice.paired = deviceProps["Paired"].toBool();
          qDebug() << "Paired status updated for:" << address << " to " << existingDevice.paired;
        }
        if (existingDevice.connected != deviceProps["Connected"].toBool()) {
          existingDevice.connected = deviceProps["Connected"].toBool();
          qDebug() << "Connected status updated for:" << address << " to " << existingDevice.connected;
        }
      }
    }
  }

  // Slot for when a device connects
  void BluetoothHandler::onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();

      m_connectedDeviceCount++;
      emit connectedDeviceCountChanged();

      auto it = std::find_if(m_devices.begin(), m_devices.end(),
                             [&path](const BluetoothDevice& device) { return device.path == path; });

      m_activeDeviceIndex = std::distance(m_devices.begin(), it);

      emit activeDeviceIndexChanged();

      qDebug() << "Device connected:" << properties.value("Address").toString();
      m_bluetoothConnectionStatus = BluetoothConnectionStatus::Value::BC_CONNECTED;
      emit bluetoothConnnectionStatusChanged();
    }
  }

// Slot for when a device disconnects
  void BluetoothHandler::onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      m_connectedDeviceCount--;
      emit connectedDeviceCountChanged();

      m_activeDeviceIndex = -1;
      emit activeDeviceIndexChanged();

      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      m_bluetoothConnectionStatus = BluetoothConnectionStatus::Value::BC_DISCONNECTED;
      emit bluetoothConnnectionStatusChanged();
    }
  }


}