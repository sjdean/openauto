#include "f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp"
#include <aaw/MessageId.pb.h>

#include <service/bluetooth/message/BluetoothPairingRequest.pb.h>
#include <service/control/message/ByeByeRequest.pb.h>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBtHandler, "journeyos.bluetooth")

namespace f1x::openauto::autoapp::UI::Monitor {
    /**
     * Interface between the UI and Local Hardware. To listen for Device Connectivity and advise UI when those details change.
     * @param configuration
     * @param parent
     */
    BluetoothHandler::BluetoothHandler(configuration::IConfiguration::Pointer configuration, QObject *parent)
        : QObject(parent)
          , configuration_(std::move(configuration))
#ifdef Q_OS_LINUX
    , m_manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus())
#endif
    {
        // 1. Cross-Platform Hardware Init (Power/Visibility)
        // This works on Mac AND Linux. No D-Bus needed.
        localDevice_ = std::make_unique<QBluetoothLocalDevice>();
        if (localDevice_->isValid()) {
            localDevice_->powerOn();
            localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);

            // Listen for pairing completion (Qt handles the handshake)
            connect(localDevice_.get(), &QBluetoothLocalDevice::pairingFinished,
                    this, &BluetoothHandler::onPairingFinished);

            qInfo(lcBtHandler) << "System Bluetooth Initialized. Address: "
                    << localDevice_->address().toString().toStdString();
        } else {
            qCritical(lcBtHandler) << "Bluetooth Adapter is not valid.";
        }

        // 2. Cross-Platform Scanning (Replaces manual D-Bus scanning)
        discoveryAgent_ = new QBluetoothDeviceDiscoveryAgent(this);
        discoveryAgent_->setLowEnergyDiscoveryTimeout(5000);

        connect(discoveryAgent_, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &BluetoothHandler::onDeviceDiscovered);
        connect(discoveryAgent_, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &BluetoothHandler::onScanFinished);

        // 3. LINUX AGENT (Strictly for Linux Pairing Popups)
#ifdef Q_OS_LINUX
        qInfo(lcBtHandler) << "Registering Custom BlueZ Agent...";
        m_agent = new BluetoothAgent("/uk/co/cubeone/journeyos/agent", this);

        QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus());
        if (agentManager.isValid()) {
            agentManager.call("RegisterAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())),
                              "DisplayYesNo");
            agentManager.call("RequestDefaultAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())));
        }

        // Connect Agent signals to this Handler
        connect(m_agent, &BluetoothAgent::pinConfirmationRequested,
                this, &BluetoothHandler::onAgentPinRequested);
#endif
    }

    void BluetoothHandler::startScan() {
        qInfo(lcBtHandler) << "Starting Bluetooth Scan...";
        m_devices.clear(); // Clear internal list
        Q_EMIT unpairedDeviceListChanged(); // Notify UI to clear list

        discoveryAgent_->start();
    }

    void BluetoothHandler::pair(const QString &address) {
        qInfo(lcBtHandler) << "Initiating pair with: " << address.toStdString();

        // Cross-platform pairing request.
        // Mac: Triggers system dialog.
        // Linux: Triggers our BluetoothAgent.
        localDevice_->requestPairing(QBluetoothAddress(address), QBluetoothLocalDevice::Paired);
    }

    void BluetoothHandler::onDeviceDiscovered(const QBluetoothDeviceInfo &info) {
        // Filter out Low Energy (if you only want Classic Audio)
        if (!(info.coreConfigurations() & QBluetoothDeviceInfo::BaseRateCoreConfiguration)) {
            return;
        }

        QString address = info.address().toString();
        QString name = info.name();

        // Avoid duplicates
        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it == m_devices.end()) {
            // On Mac, we don't have a D-Bus path, so we pass an empty one.
#ifdef Q_OS_LINUX
            QDBusObjectPath path("/"); // Placeholder, or you could query BlueZ if needed
#else
            QDBusObjectPath path;
#endif

            Model::BluetoothDevice device(address, name, path, false, false);
            m_devices.append(device);

            Q_EMIT unpairedDeviceListChanged();
            qDebug(lcBtHandler) << "Found device: " << name.toStdString();
        }
    }

    void BluetoothHandler::onScanFinished() {
        qInfo(lcBtHandler) << "Scan Finished. Found " << m_devices.size() << " devices.";
    }

    void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
        if (pairing == QBluetoothLocalDevice::Paired || pairing == QBluetoothLocalDevice::AuthorizedPaired) {
            qInfo(lcBtHandler) << "Pairing Successful: " << address.toString().toStdString();

            configuration_->updateSettingByName("Bluetooth", "PairedDeviceAddress", address.toString());
            configuration_->save();

            // Update internal list status
            auto it = std::find_if(m_devices.begin(), m_devices.end(),
                                   [&address](const Model::BluetoothDevice &d) {
                                       return d.address == address.toString();
                                   });
            if (it != m_devices.end()) {
                it->paired = true;
                Q_EMIT pairedDeviceListChanged();
            }
        } else {
            qWarning(lcBtHandler) << "Pairing Failed for: " << address.toString().toStdString();
        }
    }

    // --- Linux Agent Handlers ---

#ifdef Q_OS_LINUX
    void BluetoothHandler::onAgentPinRequested(const QString &pin, const QString &deviceAddress) {
        qInfo(lcBtHandler) << "PIN Confirmation Required: " << pin.toStdString();
        Q_EMIT pairingPinConfirmation(pin, deviceAddress);
    }
#endif

    /** Older Function **/

    QVariantList BluetoothHandler::getBluetoothAdapterList() {
        QVariantList model;

        // 1. Get all physical adapters known to Qt
        // Linux: Returns [hci0, hci1, ...]
        // Mac: Returns [Default Adapter]
        QList<QBluetoothHostInfo> hostInfos = QBluetoothLocalDevice::allDevices();

        for (const QBluetoothHostInfo &info: hostInfos) {
            QVariantMap map;
            map["name"] = info.name();
            map["address"] = info.address().toString();

            // 2. Probe the status of THIS specific adapter
            QBluetoothLocalDevice adapter(info.address());
            if (adapter.isValid()) {
                map["powered"] = (adapter.hostMode() != QBluetoothLocalDevice::HostPoweredOff);
                map["discoverable"] = (adapter.hostMode() == QBluetoothLocalDevice::HostDiscoverable);
            } else {
                map["powered"] = false;
                map["discoverable"] = false;
            }

            // 3. Fill 'path' and 'discovering' for UI compatibility
            // (We don't use 'path' for logic anymore, but QML might expect the key)
#ifdef Q_OS_LINUX
            // On BlueZ, name is usually "hci0", so path is likely "/org/bluez/hci0"
            map["path"] = "/org/bluez/" + info.name();
#else
            map["path"] = "";
#endif
            map["discovering"] = false; // Qt API doesn't expose "is discovering" for non-active adapters

            model.append(map);
        }

        return model;
    }

    QString BluetoothHandler::getAdapterAddress() const {
        if (localDevice_ && localDevice_->isValid()) {
            return localDevice_->address().toString();
        }
        return QString();
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
        if (m_activeDeviceIndex < 0) return true;

#ifdef Q_OS_LINUX
        // Linux: Force disconnect via D-Bus
        // We reconstruct the path if it's missing: "/org/bluez/hci0/dev_XX_XX_XX..."
        QString pathStr = m_devices[m_activeDeviceIndex].path.path();
        if (pathStr.isEmpty() || pathStr == "/") {
            QString addr = m_devices[m_activeDeviceIndex].address;
            addr.replace(":", "_");
            pathStr = "/org/bluez/hci0/dev_" + addr;
        }

        QDBusInterface deviceInterface("org.bluez", pathStr, "org.bluez.Device1", QDBusConnection::systemBus());
        QDBusReply<void> reply = deviceInterface.call("Disconnect");
        return reply.isValid();
#else
        return true; // Mac/Windows: OS handles disconnection
#endif
    }

    /**
     * Private function to connect to Bluetooth Device by Path
     * @param device
     * @return
     */
    bool BluetoothHandler::connectToDevice(const Model::BluetoothDevice &device) {
#ifdef Q_OS_LINUX
        // Linux: Force connect via D-Bus
        QString pathStr = device.path.path();
        if (pathStr.isEmpty() || pathStr == "/") {
            QString addr = device.address;
            addr.replace(":", "_");
            pathStr = "/org/bluez/hci0/dev_" + addr;
        }

        QDBusInterface deviceInterface("org.bluez", pathStr, "org.bluez.Device1", QDBusConnection::systemBus());
        QDBusReply<void> reply = deviceInterface.call("Connect");
        return reply.isValid();
#else
        // Mac/Windows: Initiate Pairing (which usually connects)
        QBluetoothAddress btAddress(device.address);
        localDevice_->requestPairing(btAddress, QBluetoothLocalDevice::Paired);
        return true;
#endif
    }

    bool BluetoothHandler::doConnectToPairedDevice(const QString &address) {
        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it != m_devices.end()) {
            return doConnectToPairedDevice(*it);
        }

        // If device not found in scan list, try to connect anyway using address
#ifdef Q_OS_LINUX
        QDBusObjectPath dummyPath; // Can't easily construct path without scan
        Model::BluetoothDevice dummy(address, "Unknown", dummyPath, false, false);
        return doConnectToPairedDevice(dummy);
#else
        // Mac just needs address
        QDBusObjectPath dummyPath;
        Model::BluetoothDevice dummy(address, "Unknown", dummyPath, false, false);
        return doConnectToPairedDevice(dummy);
#endif
    }

    /**
     * Connects to a previously paired Bluetooth device.
     * @param device The Bluetooth device to connect to. This includes its address and other details.
     * @return True if the connection to the device was successful, false otherwise.
     */
    bool BluetoothHandler::doConnectToPairedDevice(Model::BluetoothDevice device) {
        disconnectCurrentDevice();
        configuration_->updateSettingByName("Bluetooth", "PairedDeviceAddress", device.address);
        configuration_->save();
        return connectToDevice(device);
    }

    bool BluetoothHandler::doRemovePair(const QString &address) {
        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it == m_devices.end()) {
            qWarning(lcBtHandler) << "doRemovePair: No device found with address" << address;
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
    bool BluetoothHandler::doRemovePair(const Model::BluetoothDevice &device) {
#ifdef Q_OS_LINUX
        // Linux: Remove via Adapter D-Bus

        // 1. Get the correct adapter path dynamically
        QString adapterPath = getBluezAdapterPath();

        // 2. Construct the device path dynamically if it's missing
        // Format: /org/bluez/hciX/dev_XX_XX_XX_XX_XX_XX
        QString pathStr = device.path.path();
        if (pathStr.isEmpty() || pathStr == "/") {
            QString addr = device.address;
            addr.replace(":", "_");
            pathStr = adapterPath + "/dev_" + addr;
        }

        // 3. Call RemoveDevice on the correct adapter interface
        QDBusInterface adapterInterface("org.bluez", adapterPath, "org.bluez.Adapter1", QDBusConnection::systemBus());
        QDBusReply<void> reply = adapterInterface.call("RemoveDevice", QVariant::fromValue(QDBusObjectPath(pathStr)));

        return reply.isValid();
#else
        // Mac/Windows: Unpair via Qt
        localDevice_->requestPairing(QBluetoothAddress(device.address), QBluetoothLocalDevice::Unpaired);
        return true;
#endif
    }

    bool BluetoothHandler::doRemoveAllPairs() {
        disconnectCurrentDevice();

        // We iterate over our internal list which is simpler
        bool allSuccess = true;

        // Copy list to avoid iterator invalidation issues while removing
        QList<Model::BluetoothDevice> devicesCopy = m_devices;

        for (const auto &device: devicesCopy) {
            if (device.paired) {
                if (!doRemovePair(device)) {
                    allSuccess = false;
                }
            }
        }

        configuration_->updateSettingByName("Bluetooth", "PairedDeviceAddress", "");
        configuration_->save();
        return allSuccess;
    }

    common::Enum::BluetoothConnectionStatus::Value BluetoothHandler::getBluetoothConnectionStatus() const {
        return m_bluetoothConnectionStatus;
    }

    void BluetoothHandler::setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::Value value) {
        m_bluetoothConnectionStatus = value;
        Q_EMIT bluetoothConnnectionStatusChanged();
    }

    QString BluetoothHandler::getStatusText() const {
        switch (m_bluetoothConnectionStatus) {
            case common::Enum::BluetoothConnectionStatus::BC_CONNECTED:    return QStringLiteral("Connected");
            case common::Enum::BluetoothConnectionStatus::BC_CONNECTING:   return QStringLiteral("Connecting...");
            case common::Enum::BluetoothConnectionStatus::BC_DISCONNECTED: return QStringLiteral("Disconnected");
            default:                                                        return QStringLiteral("Not Configured");
        }
    }

    QVariantList BluetoothHandler::getPairedDeviceList() {
        QVariantList model;
        for (const auto &device: m_devices) {
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
        for (const auto &device: m_devices) {
            if (!device.paired) {
                QVariantMap deviceMap;
                deviceMap.insert("name", device.name);
                deviceMap.insert("address", device.address);
                model.append(deviceMap);
            }
        }
        return model;
    }

#ifdef Q_OS_LINUX
    QString BluetoothHandler::getBluezAdapterPath() {
        // Ask BlueZ for all objects (Adapters, Devices, etc.)
        QDBusReply<QVariantMap> reply = m_manager.call("GetManagedObjects");

        if (reply.isValid()) {
            QVariantMap objects = reply.value();
            QString localAddress = localDevice_->address().toString();

            // Iterate to find the Adapter with our address
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                const QVariantMap &interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Adapter1")) {
                    QString adapterAddr = interfaces["org.bluez.Adapter1"].toMap()["Address"].toString();
                    if (adapterAddr == localAddress) {
                        return it.key(); // Returns "/org/bluez/hci0" (or hci1, etc.)
                    }
                }
            }
        }
        // Fallback if lookup fails (unlikely)
        return QStringLiteral("/org/bluez/hci0");
    }
#endif

    void BluetoothHandler::setActiveAdapter(const QString &address) {
        if (localDevice_->address().toString() == address) return;

        qInfo(lcBtHandler) << "Switching active adapter to: " << address;

        // 1. Clean up old device
        if (localDevice_) {
            localDevice_->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
        }

        // 2. Initialize new device
        localDevice_ = std::make_unique<QBluetoothLocalDevice>(QBluetoothAddress(address));
        if (localDevice_->isValid()) {
            localDevice_->powerOn();
            localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
            connect(localDevice_.get(), &QBluetoothLocalDevice::pairingFinished,
                    this, &BluetoothHandler::onPairingFinished);

            // Save preference
            configuration_->updateSettingByName("Bluetooth", "ActiveAdapter", address);
            configuration_->save();
        }
    }

    void BluetoothHandler::doDisconnect(const QString &address) {
        qInfo(lcBtHandler) << "Requesting Disconnect for device:" << address;

#ifdef Q_OS_LINUX
        QString pathStr;

        // 1. Try to find the device path in our cached model
        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it != m_devices.end()) {
            pathStr = it->path.path();
        }

        // 2. If path is missing or empty, construct it manually (Standard BlueZ format)
        // Format: /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX
        if (pathStr.isEmpty() || pathStr == "/") {
            QString cleanAddress = address;
            cleanAddress.replace(":", "_");

            // Use our helper to get the adapter path (e.g., /org/bluez/hci0)
            QString adapterPath = getBluezAdapterPath();
            pathStr = adapterPath + "/dev_" + cleanAddress;
        }

        qInfo(lcBtHandler) << "Sending DBus Disconnect to:" << pathStr;

        // 3. Call BlueZ API
        QDBusInterface deviceInterface("org.bluez", pathStr, "org.bluez.Device1", QDBusConnection::systemBus());
        deviceInterface.call(QDBus::NoBlock, "Disconnect");

#else
        // macOS/Windows: Qt Bluetooth API does not allow forcing a disconnect
        // of a system-level pairing from the app side easily.
        qInfo(lcBtHandler) << "Manual disconnect not supported on this platform via Qt API.";
#endif
    }
}
