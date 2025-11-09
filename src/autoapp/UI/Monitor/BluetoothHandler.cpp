#include "f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {
  /**
   * Handles operations related to Bluetooth functionality
   */
  BluetoothHandler::BluetoothHandler(configuration::IConfiguration::Pointer configuration, QObject *parent)
  : QObject(parent),
    configuration_(std::move(configuration)),
    m_manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus()),
    m_agent(nullptr) {

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
          Model::BluetoothAdapter adapter(adapterProps["Address"].toString(),adapterProps["Name"].toString(),it.key(),adapterProps["Powered"].toBool(),adapterProps["Discoverable"].toBool(),adapterProps["Discovering"].toBool());
          m_adapters.append(adapter);
        }

        // If Device...
        if (interfaces.contains("org.bluez.Device1")) {
          QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();

          Model::BluetoothDevice device(deviceProps["Address"].toString(), deviceProps["Name"].toString(), QDBusObjectPath(deviceProps["Path"].toString()), deviceProps["Paired"].toBool(), deviceProps["Connected"].toBool());
          m_devices.append(device);
        }
      }
      emit bluetoothAdapterListChanged();
    } else {
      qWarning() << "Failed to get managed objects:" << reply.error().message();
    }

    // Find Adapter by Hardware Address
    auto it = std::find_if(m_adapters.begin(), m_adapters.end(),
                           [this](const Model::BluetoothAdapter &adapter) {
                             return adapter.hardwareAddress == configuration_->getSettingByName<QString>("Bluetooth","AdapterAddress");
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

    m_agent = new BluetoothAgent("/org/journeyos/agent", this);

    QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus());
    if (!agentManager.isValid()) {
      qWarning() << "Bluetooth AgentManager not found.";
    } else {
      // 3. Register the Agent with "DisplayYesNo" capability
      QDBusReply<void> reply = agentManager.call("RegisterAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())), "DisplayYesNo");
      if (!reply.isValid()) {
        qWarning() << "Failed to register Bluetooth agent:" << reply.error().message();
      } else {
        // 4. Make it the default agent
        agentManager.call("RequestDefaultAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())));
        qInfo() << "Bluetooth Agent registered successfully.";
      }
    }
  }

  QVariantList BluetoothHandler::getBluetoothAdapterList() {
    QVariantList model;
    for (const auto &adapter : m_adapters) {
      QVariantMap adapterMap;
      adapterMap.insert("name", adapter.name);
      adapterMap.insert("address", adapter.hardwareAddress);
      adapterMap.insert("path", adapter.path);
      adapterMap.insert("powered", adapter.powered);
      adapterMap.insert("discoverable", adapter.discoverable);
      adapterMap.insert("discovering", adapter.discovering);
      model.append(adapterMap);
    }
    return model;
  }

  int BluetoothHandler::getConnectedDeviceCount() const {
    return m_connectedDeviceCount;
  }

  int BluetoothHandler::getAdapterCount() const {
    return m_adapters.size();
  }

  int BluetoothHandler::getActiveDeviceIndex() const {
    return m_activeDeviceIndex;
  }

  /* Action Functions */

  /**
 * Private function to disconnect from active device
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
   * @param device
   * @return
   */
  bool BluetoothHandler::connectToDevice(const Model::BluetoothDevice& device) {

    auto reply = m_manager.call("ConnectDevice", m_devices[m_activeDeviceIndex].path);

    if (reply.type() == QDBusMessage::ReplyMessage) {
      qDebug() << "Successfully connected to device: " << m_devices[m_activeDeviceIndex].address;
      return true;
    } else {
      qWarning() << "Failed to connect to device: " << m_devices[m_activeDeviceIndex].address << " - " << reply.errorMessage();
      return false;
    }
  }

  bool BluetoothHandler::doConnectToPairedDevice(const QString &address) {
    auto it = std::find_if(m_devices.begin(), m_devices.end(),
                           [&address](const Model::BluetoothDevice &d) { return d.address == address; });

    if (it == m_devices.end()) {
      qWarning() << "doConnectToPairedDevice: No device found with address" << address;
      return false;
    }

    // Now call your *original* logic (which you should make private)
    // We pass 'it->' which is a 'Model::BluetoothDevice' object
    return doConnectToPairedDevice(*it); // This now calls your original C++ function
  }

  /**
   * Connects to a previously paired Bluetooth device.
   * @param device The Bluetooth device to connect to. This includes its address and other details.
   * @return True if the connection to the device was successful, false otherwise.
   */
  bool BluetoothHandler::doConnectToPairedDevice(Model::BluetoothDevice device) {
    // Disconnect from current device if connected
    disconnectCurrentDevice();
    configuration_->updateSettingByName("Bluetooth","PairedDeviceAddress", device.address);

    // Connect to selected device
    if (connectToDevice(std::move(device))) {
      return true;
      // Handle successful connection
    } else {
      return false;
      // Handle connection failure
    }
  }

  bool BluetoothHandler::doRemovePair(const QString &address) {
    auto it = std::find_if(m_devices.begin(), m_devices.end(),
                           [&address](const Model::BluetoothDevice &d) { return d.address == address; });

    if (it == m_devices.end()) {
      qWarning() << "doRemovePair: No device found with address" << address;
      return false;
    }

    // Now call your *original* logic
    return doRemovePair(*it); // This calls your original C++ function
  }

  /**
   * Removes the pairing of a specified Bluetooth device. If the removed device is the
   * currently active device, it will be disconnected. Updates the configuration settings
   * if the removed device matches the currently stored paired device.
   *
   * @param device The Bluetooth device to be unpaired, including its address and path.
   * @return True if the device was successfully unpaired, false otherwise.
   */
  bool BluetoothHandler::doRemovePair(const Model::BluetoothDevice& device) {
    // Construct the object path for the device

    QDBusReply<void> reply = m_manager.call("RemoveDevice", device.path);
    if (device.path == m_devices[m_activeDeviceIndex].path) {
      disconnectCurrentDevice();
    }

    if (reply.isValid()) {
      if (configuration_->getSettingByName<QString>("Bluetooth","PairedDeviceAddress") == device.address) {
        configuration_->updateSettingByName("Bluetooth","PairedDeviceAddress", "");
      }
      qDebug() << "Successfully unpaired device: " << device.path.path();

      return true;
    } else {
      qWarning() << "Failed to unpair device: " << reply.error().message();
      return false;
    }
  }

  /**
   * Removes all paired Bluetooth devices by disconnecting the current device,
   * fetching the list of devices from the adapter, and removing each device.
   * Updates the configuration settings after removing the devices.
   * @return True if all devices were successfully removed, false if any removal failed.
   */
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

    configuration_->updateSettingByName("Bluetooth","PairedDeviceAddress", "");
    return allSuccess;
  }

  /**
   * Initiates a scan for available Bluetooth devices to pair with.
   * Stops any ongoing discovery process before starting a new one. Filters out
   * paired devices from the unpaired device list and triggers the unpairedDeviceListChanged signal.
   * Starts a timer to optionally stop the discovery process after a predefined duration.
   *
   * @return True if the discovery process started successfully, false otherwise.
   */
  bool BluetoothHandler::doScanDevicesForPairing() {
    // Stop any previous discovery first
    QDBusReply<void> stopReply = m_manager.call("StopDiscovery");
    if (!stopReply.isValid()) {
      qWarning() << "Failed to stop previous discovery: " << stopReply.error().message();
    }

    m_devices.erase(
        std::remove_if(m_devices.begin(), m_devices.end(),
                       [](const Model::BluetoothDevice &device) { return !device.paired; }),
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

  Enum::BluetoothConnectionStatus::Value BluetoothHandler::getBluetoothConnectionStatus() const {
    return m_bluetoothConnectionStatus;
  }

  void BluetoothHandler::setBluetoothConnectionStatus(Enum::BluetoothConnectionStatus::Value value) {
    m_bluetoothConnectionStatus = value;
    emit bluetoothConnnectionStatusChanged();
  }

  QVariantList BluetoothHandler::getPairedDeviceList() {
    QVariantList model;
    for (const auto &device : m_devices) {
      if (device.paired) {
        // Create a map that QML can read
        QVariantMap deviceMap;
        deviceMap.insert("name", device.name);
        deviceMap.insert("address", device.address);
        deviceMap.insert("connected", device.connected);
        model.append(deviceMap);
      }
    }
    return model;
  }

  QVariantList BluetoothHandler::getUnpairedDeviceList() {
    QVariantList model;
    for (const auto &device : m_devices) {
      if (!device.paired) {
        QVariantMap deviceMap;
        deviceMap.insert("name", device.name);
        deviceMap.insert("address", device.address);
        model.append(deviceMap);
      }
    }
    return model;
  }

  /**
   * Handles the addition of new D-Bus interfaces and updates the internal device list.
   * If a new Bluetooth device is detected, it is added to the device list. If an existing
   * device is updated, its properties are refreshed.
   *
   * @param objectPath The D-Bus object path representing the device.
   * @param interfacesAndProperties The map of interfaces and their associated properties.
   */
  void BluetoothHandler::onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties) {
    if (interfacesAndProperties.contains("org.bluez.Device1")) {
      QVariantMap deviceProps = interfacesAndProperties["org.bluez.Device1"].toMap();
      QString address = deviceProps.value("Address").toString();
      QString name = deviceProps.value("Name").toString();

      // Check if this is a new device or an update to an existing one
      auto it = std::find_if(m_devices.begin(), m_devices.end(),
                             [&address](const Model::BluetoothDevice& device) { return device.address == address; });

      if (it == m_devices.end()) {
        // New device
        Model::BluetoothDevice device(deviceProps["Address"].toString(), deviceProps["Name"].toString(),
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
        Model::BluetoothDevice &existingDevice = *it;
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

  void BluetoothHandler::onDeviceFound(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties)
  {
    // This function will receive a map of interfaces. We only care about the Device1 interface.
    if (!interfacesAndProperties.contains("org.bluez.Device1")) {
      return;
    }

    QVariantMap deviceProps = interfacesAndProperties["org.bluez.Device1"].toMap();
    QString address = deviceProps.value("Address").toString();
    QString name = deviceProps.value("Name").toString();

    // Check if we already have this device in our list
    auto it = std::find_if(m_devices.begin(), m_devices.end(),
                           [&address](const Model::BluetoothDevice& device) { return device.address == address; });

    if (it == m_devices.end()) {
      // --- This is a new device ---
      qDebug() << "New device found:" << address << " - " << name;

      // Create the new device from the properties
      Model::BluetoothDevice device(
          address,
          name,
          objectPath, // Use the objectPath from the signal
          deviceProps.value("Paired").toBool(),
          deviceProps.value("Connected").toBool()
      );

      m_devices.append(device);

      // Notify the QML of the new device
      if (device.paired) {
        emit pairedDeviceListChanged();
      } else {
        // This is the signal that will update your "scan results" list in QML
        emit unpairedDeviceListChanged();
      }
    }
    else
    {
      // We already have this device, but maybe its properties updated (e.g., name)
      it->name = name;
      it->paired = deviceProps.value("Paired").toBool();
      it->connected = deviceProps.value("Connected").toBool();

      // Emit both, just in case (simpler)
      emit pairedDeviceListChanged();
      emit unpairedDeviceListChanged();
    }
  }

  /**
   * Handles the connection of a Bluetooth device.
   * Updates the connected device count, the active device index, and connection status.
   * Emits relevant signals to notify changes in the Bluetooth state.
   *
   * @param path The D-Bus object path representing the connected device.
   */
  void BluetoothHandler::onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();

      m_connectedDeviceCount++;
      emit connectedDeviceCountChanged();

      auto it = std::find_if(m_devices.begin(), m_devices.end(),
                             [&path](const Model::BluetoothDevice& device) { return device.path == path; });

      m_activeDeviceIndex = std::distance(m_devices.begin(), it);

      emit activeDeviceIndexChanged();

      if (it != m_devices.end()) {
        it->connected = true; // Mark as disconnected
        emit pairedDeviceListChanged(); // Notify QML
      }

      qDebug() << "Device connected:" << properties.value("Address").toString();
      m_bluetoothConnectionStatus = Enum::BluetoothConnectionStatus::Value::BC_CONNECTED;
      emit bluetoothConnnectionStatusChanged();
    }
  }

  /**
   * Handles the disconnection of a Bluetooth device.
   * Updates the connected device count, the active device index, and connection status.
   * Emits relevant signals to notify changes in the Bluetooth state.
   *
   * @param path The D-Bus object path representing the disconnected device.
   */
  void BluetoothHandler::onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      m_connectedDeviceCount--;
      emit connectedDeviceCountChanged();

      m_activeDeviceIndex = -1;
      emit activeDeviceIndexChanged();

      auto it = std::find_if(m_devices.begin(), m_devices.end(),
                       [&path](const Model::BluetoothDevice& device) { return device.path == path; });

      if (it != m_devices.end()) {
        it->connected = false; // Mark as disconnected
        emit pairedDeviceListChanged(); // Notify QML
      }

      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      m_bluetoothConnectionStatus = Enum::BluetoothConnectionStatus::Value::BC_DISCONNECTED;
      emit bluetoothConnnectionStatusChanged();
    }
  }
}