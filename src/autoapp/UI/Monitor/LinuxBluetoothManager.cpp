#include "f1x/openauto/autoapp/UI/Monitor/LinuxBluetoothManager.hpp"
#include <aaw/MessageId.pb.h>

#ifdef Q_OS_LINUX
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFileInfo>
#endif

#include <aap_protobuf/service/bluetooth/message/BluetoothPairingRequest.pb.h>
#include <aap_protobuf/service/control/message/ByeByeRequest.pb.h>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#include <QTimer>
#include <QDir>
#include <QFile>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBtHandler, "journeyos.bluetooth")

// Returns a human-readable hardware label for the BT adapter that owns `address`.
// Strategy:
//   1. Use BlueZ GetManagedObjects to map MAC → hci name (e.g. "hci1")
//   2. Resolve /sys/class/bluetooth/hci1/device symlink → USB interface path
//   3. Read the parent USB device's "product" string (e.g. "CSR8510 A10")
//   4. For built-in UART adapters (no USB product) return "Built-in"
static QString btVendorName(const QBluetoothAddress &address)
{
#ifdef Q_OS_LINUX
    const QString targetAddr = address.toString().toUpper();

    // Step 1: ask BlueZ which hci object path belongs to this MAC
    QDBusInterface om("org.bluez", "/",
                      "org.freedesktop.DBus.ObjectManager",
                      QDBusConnection::systemBus());
    const QDBusMessage reply = om.call("GetManagedObjects");
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return {};

    using ObjectMap = QMap<QDBusObjectPath, QMap<QString, QVariantMap>>;
    const auto objects = qdbus_cast<ObjectMap>(reply.arguments().at(0));

    QString hciName;
    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        if (!it.value().contains(QLatin1String("org.bluez.Adapter1")))
            continue;
        const QString addr = it.value()[QLatin1String("org.bluez.Adapter1")]
                             [QLatin1String("Address")].toString().toUpper();
        if (addr == targetAddr) {
            // Object path is "/org/bluez/hci0" — take the last segment
            hciName = it.key().path().section(QLatin1Char('/'), -1);
            break;
        }
    }
    if (hciName.isEmpty())
        return {};

    // Step 2: resolve /sys/class/bluetooth/hciN/device symlink
    const QString deviceLink = QStringLiteral("/sys/class/bluetooth/") + hciName
                               + QStringLiteral("/device");
    const QString resolvedIface = QFileInfo(deviceLink).canonicalFilePath();

    if (!resolvedIface.isEmpty()) {
        // Step 3: parent dir = the USB device node (one level above the interface)
        const QString usbDevPath = QFileInfo(resolvedIface).canonicalPath();

        QFile productFile(usbDevPath + QStringLiteral("/product"));
        if (productFile.open(QIODevice::ReadOnly)) {
            const QString product = QString::fromLatin1(productFile.readAll().trimmed());
            if (!product.isEmpty())
                return product;           // e.g. "CSR8510 A10"
        }

        QFile mfFile(usbDevPath + QStringLiteral("/manufacturer"));
        if (mfFile.open(QIODevice::ReadOnly)) {
            const QString mf = QString::fromLatin1(mfFile.readAll().trimmed());
            if (!mf.isEmpty())
                return mf;
        }
    }

    // Step 4: no USB product string → built-in UART adapter
    return QStringLiteral("Built-in");
#else
    Q_UNUSED(address)
    return {};
#endif
}

namespace f1x::openauto::autoapp::UI::Monitor {
using configuration::ConfigGroup;
using configuration::ConfigKey;
    /**
     * Interface between the UI and Local Hardware. To listen for Device Connectivity and advise UI when those details change.
     * @param configuration
     * @param parent
     */
    LinuxBluetoothManager::LinuxBluetoothManager(configuration::IConfiguration::Pointer configuration, QObject *parent)
        : IBluetoothManager(parent)
          , configuration_(std::move(configuration))
#ifdef Q_OS_LINUX
    , m_manager("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus())
#endif
    {
#ifdef Q_OS_LINUX
        qDBusRegisterMetaType<BluezInterfaceList>();
        qDBusRegisterMetaType<BluezManagedObjects>();
#endif
        // 1. Restore saved adapter preference; fall back to system default
        const QString savedAdapter = configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress);
        if (!savedAdapter.isEmpty()) {
            localDevice_ = std::make_unique<QBluetoothLocalDevice>(QBluetoothAddress(savedAdapter));
            if (!localDevice_->isValid()) {
                qWarning(lcBtHandler) << "saved adapter not available using default adapter=" << savedAdapter;
                localDevice_ = std::make_unique<QBluetoothLocalDevice>();
            }
        } else {
            localDevice_ = std::make_unique<QBluetoothLocalDevice>();
        }

        if (localDevice_->isValid()) {
            localDevice_->powerOn();
            localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
            connect(localDevice_.get(), &QBluetoothLocalDevice::pairingFinished,
                    this, &LinuxBluetoothManager::onPairingFinished);
            // Persist auto-detected adapter so future boots use the same one
            if (savedAdapter.isEmpty()) {
                const QString addr = localDevice_->address().toString();
                configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress, addr);
                configuration_->save();
                qInfo(lcBtHandler) << "adapter auto-detected and saved address=" << addr;
            } else {
                qInfo(lcBtHandler) << "adapter ready address=" << localDevice_->address().toString();
            }
        } else {
            qCritical(lcBtHandler) << "no valid adapter found";
        }

        // 2. Load ignored-device list from config
        const QString ignored = configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothIgnoredDevices);
        if (!ignored.isEmpty())
            m_ignoredDevices = ignored.split(',', Qt::SkipEmptyParts);

        // 3. Cross-platform scanning
        discoveryAgent_ = new QBluetoothDeviceDiscoveryAgent(this);
        discoveryAgent_->setLowEnergyDiscoveryTimeout(5000);
        connect(discoveryAgent_, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &LinuxBluetoothManager::onDeviceDiscovered);
        connect(discoveryAgent_, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                this, [this](const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields) {
                    // Device names often arrive after initial discovery — update existing entry
                    const QString address = info.address().toString();
                    const QString name = info.name();
                    if (name.isEmpty()) return;
                    for (auto &device : m_devices) {
                        if (device.address == address && device.name != name) {
                            device.name = name;
                            Q_EMIT unpairedDeviceListChanged();
                            Q_EMIT pairedDeviceListChanged();
                            break;
                        }
                    }
                });
        connect(discoveryAgent_, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &LinuxBluetoothManager::onScanFinished);

        // 4. Linux BlueZ pairing agent
#ifdef Q_OS_LINUX
        qDebug(lcBtHandler) << "registering bluez agent";
        m_agent = new BluetoothAgent("/uk/co/cubeone/journeyos/agent", this);
        QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus());
        if (agentManager.isValid()) {
            agentManager.call("RegisterAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())),
                              "DisplayYesNo");
            agentManager.call("RequestDefaultAgent", QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())));
        }
        connect(m_agent, &BluetoothAgent::showConfirmation,
                this, [this](const QString &passkey) {
                    onAgentPinRequested(passkey, QString());
                });
        connect(m_agent, &BluetoothAgent::showPinCode,
                this, [this](const QString &pin) {
                    onAgentPinRequested(pin, QString());
                });
        connect(m_agent, &BluetoothAgent::pairingComplete,
                this, []() { /* pairing complete — QML dismisses popup via state change */ });
#endif

        // 5. Load already-paired devices from BlueZ so the list is populated on startup
#ifdef Q_OS_LINUX
        loadPairedDevicesFromBlueZ();
        // Cache adapter path now so onDeviceDiscovered can use it without a D-Bus round-trip
        m_cachedAdapterPath = getBluezAdapterPath();
        // Subscribe to InterfacesAdded — fires when BlueZ creates a new device object,
        // which includes the Name property if BlueZ has already resolved it.
        subscribeToInterfacesAdded();
#endif

        // Notify QML that the adapter list is available
        Q_EMIT bluetoothAdapterListChanged();
        Q_EMIT adapterCountChanged();

        // 6. Auto-connect to last known device after the BT stack settles.
        //    If the saved device is unreachable, fall back to other paired devices.
        QTimer::singleShot(3000, this, [this]() {
            const QString lastDevice = configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothPairedDeviceAddress);
            if (!lastDevice.isEmpty()) {
                qInfo(lcBtHandler) << "auto-connect device=" << lastDevice;
                if (connectToDevice(lastDevice)) return;
                qWarning(lcBtHandler) << "auto-connect failed device=" << lastDevice << " trying others";
            }
            for (const auto &device : m_devices) {
                if (device.paired && device.address != lastDevice) {
                    qInfo(lcBtHandler) << "auto-connect fallback device=" << device.address;
                    if (connectToDevice(device.address)) return;
                }
            }
        });
    }

    void LinuxBluetoothManager::startScan() {
        qInfo(lcBtHandler) << "scan starting";
        // Only clear unpaired/discovered devices — keep paired entries intact
        m_devices.erase(std::remove_if(m_devices.begin(), m_devices.end(),
            [](const Model::BluetoothDevice &d) { return !d.paired; }), m_devices.end());
        Q_EMIT unpairedDeviceListChanged();
        m_isScanning = true;
        Q_EMIT isScanningChanged();
        discoveryAgent_->start();

        // Classic BT (BR/EDR) has no built-in timeout — stop after 12 seconds if still running.
        // BLE already stops via setLowEnergyDiscoveryTimeout(5000).
        QTimer::singleShot(12000, this, [this]() {
            if (m_isScanning)
                discoveryAgent_->stop();
        });
    }

    bool LinuxBluetoothManager::isScanning() const {
        return m_isScanning;
    }

    void LinuxBluetoothManager::pair(const QString &address) {
        qInfo(lcBtHandler) << "pairing address=" << address;

        // Cross-platform pairing request.
        // Mac: Triggers system dialog.
        // Linux: Triggers our BluetoothAgent.
        localDevice_->requestPairing(QBluetoothAddress(address), QBluetoothLocalDevice::Paired);
    }

    void LinuxBluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info) {
        QString address = info.address().toString();
        QString name = info.name();

        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it == m_devices.end()) {
            // New device
#ifdef Q_OS_LINUX
            Model::BluetoothDevice device(address, name, QDBusObjectPath("/"), false, false);
#else
            Model::BluetoothDevice device(address, name, QString{}, false, false);
#endif
            m_devices.append(device);
            Q_EMIT unpairedDeviceListChanged();
            qDebug(lcBtHandler) << "device found address=" << address << " name=" << (name.isEmpty() ? "(no name yet)" : name);
        } else if (!name.isEmpty() && it->name.isEmpty()) {
            // Same address seen again — Classic BT inquiry response arrived after the
            // initial BLE advertisement. Qt calls deviceDiscovered a second time with
            // the full info (including name). Update the existing entry.
            it->name = name;
            Q_EMIT unpairedDeviceListChanged();
            Q_EMIT pairedDeviceListChanged();
            qDebug(lcBtHandler) << "device name resolved address=" << address << " name=" << name;
        }
    }

    void LinuxBluetoothManager::onScanFinished() {
        qInfo(lcBtHandler) << "scan finished count=" << m_devices.size();
        m_isScanning = false;
        Q_EMIT isScanningChanged();
#ifdef Q_OS_LINUX
        // Immediate pass: fill names already in BlueZ cache
        refreshDeviceNamesFromBlueZ();
        // BlueZ Remote Name Request (RNR) can take 2–10 s after the scan.
        // Schedule follow-up passes so names that resolve later are captured.
        QTimer::singleShot(3000, this, &LinuxBluetoothManager::refreshDeviceNamesFromBlueZ);
        QTimer::singleShot(8000, this, &LinuxBluetoothManager::refreshDeviceNamesFromBlueZ);
#endif
    }

    void LinuxBluetoothManager::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
        if (pairing == QBluetoothLocalDevice::Paired || pairing == QBluetoothLocalDevice::AuthorizedPaired) {
            qInfo(lcBtHandler) << "pairing successful address=" << address.toString();

            configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothPairedDeviceAddress, address.toString());
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
            qWarning(lcBtHandler) << "pairing failed address=" << address.toString();
        }
    }

    // --- Linux Agent Handlers ---

#ifdef Q_OS_LINUX
    void LinuxBluetoothManager::onAgentPinRequested(const QString &pin, const QString &deviceAddress) {
        qDebug(lcBtHandler) << "pin confirmation requested";
        Q_EMIT pairingPinConfirmation(pin, deviceAddress);
    }
#endif

    /** Older Function **/

    QVariantList LinuxBluetoothManager::getBluetoothAdapterList() {
        QVariantList model;

        // 1. Get all physical adapters known to Qt
        // Linux: Returns [hci0, hci1, ...]
        // Mac: Returns [Default Adapter]
        QList<QBluetoothHostInfo> hostInfos = QBluetoothLocalDevice::allDevices();

        for (const QBluetoothHostInfo &info: hostInfos) {
            QVariantMap map;
            const QString addrStr = info.address().toString();
            const QString vendor = btVendorName(info.address());
            const QString displayName = info.name().isEmpty() ? addrStr : info.name();
            map["name"] = vendor.isEmpty()
                          ? displayName + QStringLiteral(" \u2014 ") + addrStr
                          : vendor + QStringLiteral(" (") + displayName + QStringLiteral(") \u2014 ") + addrStr;
            map["address"] = addrStr;

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

    QString LinuxBluetoothManager::getAdapterAddress() const {
        if (localDevice_ && localDevice_->isValid()) {
            return localDevice_->address().toString();
        }
        return QString();
    }

    int LinuxBluetoothManager::getConnectedDeviceCount() const {
        return m_connectedDeviceCount;
    }

    int LinuxBluetoothManager::getAdapterCount() const {
        return QBluetoothLocalDevice::allDevices().count();
    }

    int LinuxBluetoothManager::getActiveDeviceIndex() const {
        return m_activeDeviceIndex;
    }

    /* Action Functions */

    /**
   * Private function to disconnect from active device
   * @return
   */
    bool LinuxBluetoothManager::disconnectCurrentDevice() {
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
    bool LinuxBluetoothManager::connectToDeviceImpl(const Model::BluetoothDevice &device) {
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

    bool LinuxBluetoothManager::connectToDevice(const QString &address) {
        setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::BC_CONNECTING);

        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        Model::BluetoothDevice device = (it != m_devices.end())
            ? *it
#ifdef Q_OS_LINUX
            : Model::BluetoothDevice(address, "Known Device", QDBusObjectPath(""), true, false);
#else
            : Model::BluetoothDevice(address, "Known Device", QString{}, true, false);
#endif

        const bool success = connectToPairedDeviceImpl(device);

        if (success) {
            if (it != m_devices.end())
                it->connected = true;
            setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::BC_CONNECTED);
            Q_EMIT pairedDeviceListChanged();
        } else {
            setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::BC_DISCONNECTED);
        }

        return success;
    }

    /**
     * Connects to a previously paired Bluetooth device.
     * @param device The Bluetooth device to connect to. This includes its address and other details.
     * @return True if the connection to the device was successful, false otherwise.
     */
    bool LinuxBluetoothManager::connectToPairedDeviceImpl(Model::BluetoothDevice device) {
        disconnectCurrentDevice();
        configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothPairedDeviceAddress, device.address);
        configuration_->save();
        return connectToDeviceImpl(device);
    }

    bool LinuxBluetoothManager::removePair(const QString &address) {
        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it == m_devices.end()) {
            qWarning(lcBtHandler) << "remove pair not found address=" << address;
            return false;
        }

        return removePairImpl(*it);
    }

    /**
     * Removes the pairing of a specified Bluetooth device. If the removed device is the
     * currently active device, it will be disconnected. Updates the configuration settings
     * if the removed device matches the currently stored paired device.
     *
     * @param device The Bluetooth device to be unpaired, including its address and path.
     * @return True if the device was successfully unpaired, false otherwise.
     */
    bool LinuxBluetoothManager::removePairImpl(const Model::BluetoothDevice &device) {
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

    bool LinuxBluetoothManager::removeAllPairs() {
        disconnectCurrentDevice();

        // We iterate over our internal list which is simpler
        bool allSuccess = true;

        // Copy list to avoid iterator invalidation issues while removing
        QList<Model::BluetoothDevice> devicesCopy = m_devices;

        for (const auto &device: devicesCopy) {
            if (device.paired) {
                if (!removePairImpl(device)) {
                    allSuccess = false;
                }
            }
        }

        configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothPairedDeviceAddress, QString(""));
        configuration_->save();
        return allSuccess;
    }

    common::Enum::BluetoothConnectionStatus::Value LinuxBluetoothManager::getBluetoothConnectionStatus() const {
        return m_bluetoothConnectionStatus;
    }

    void LinuxBluetoothManager::setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::Value value) {
        m_bluetoothConnectionStatus = value;
        Q_EMIT bluetoothConnectionStatusChanged();
    }

    void LinuxBluetoothManager::ignoreDevice(const QString &address) {
        if (m_ignoredDevices.contains(address))
            return;
        m_ignoredDevices.append(address);
        configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothIgnoredDevices, m_ignoredDevices.join(','));
        configuration_->save();
        Q_EMIT unpairedDeviceListChanged();
        qInfo(lcBtHandler) << "device ignored address=" << address;
    }

    QString LinuxBluetoothManager::getStatusText() const {
        switch (m_bluetoothConnectionStatus) {
            case common::Enum::BluetoothConnectionStatus::BC_CONNECTED:    return QStringLiteral("Connected");
            case common::Enum::BluetoothConnectionStatus::BC_CONNECTING:   return QStringLiteral("Connecting...");
            case common::Enum::BluetoothConnectionStatus::BC_DISCONNECTED: return QStringLiteral("Disconnected");
            default:                                                        return QStringLiteral("Not Configured");
        }
    }

    QString LinuxBluetoothManager::getConnectedDeviceName() const {
        for (const auto &device : m_devices) {
            if (device.connected)
                return device.name;
        }
        return QString();
    }

    QVariantList LinuxBluetoothManager::getPairedDeviceList() {
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

    QVariantList LinuxBluetoothManager::getUnpairedDeviceList() {
        QVariantList model;
        for (const auto &device: m_devices) {
            if (!device.paired && !m_ignoredDevices.contains(device.address)) {
                QVariantMap deviceMap;
                deviceMap.insert("name", device.name);
                deviceMap.insert("address", device.address);
                model.append(deviceMap);
            }
        }
        return model;
    }

#ifdef Q_OS_LINUX
    QString LinuxBluetoothManager::getBluezAdapterPath() {
        QDBusReply<BluezManagedObjects> reply = m_manager.call("GetManagedObjects");
        if (!reply.isValid()) {
            qWarning(lcBtHandler) << "getBluezAdapterPath get managed objects failed error=" << reply.error().message();
            return QStringLiteral("/org/bluez/hci0");
        }

        const BluezManagedObjects &objects = reply.value();
        const QString localAddress = localDevice_->address().toString();

        for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
            const BluezInterfaceList &interfaces = it.value();
            if (interfaces.contains("org.bluez.Adapter1")) {
                const QString adapterAddr = interfaces["org.bluez.Adapter1"]["Address"].toString();
                if (adapterAddr == localAddress)
                    return it.key().path();
            }
        }
        return QStringLiteral("/org/bluez/hci0");
    }

    void LinuxBluetoothManager::loadPairedDevicesFromBlueZ() {
        QDBusReply<BluezManagedObjects> reply = m_manager.call("GetManagedObjects");
        if (!reply.isValid()) {
            qWarning(lcBtHandler) << "loadPairedDevicesFromBlueZ get managed objects failed error=" << reply.error().message();
            return;
        }

        const BluezManagedObjects &objects = reply.value();
        bool changed = false;

        for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
            const BluezInterfaceList &interfaces = it.value();
            if (!interfaces.contains("org.bluez.Device1")) continue;

            const QVariantMap &props = interfaces["org.bluez.Device1"];
            if (!props.value("Paired", false).toBool()) continue;

            const QString address = props.value("Address").toString();
            const QString name = props.value("Name", props.value("Alias", address)).toString();
            const bool connected = props.value("Connected", false).toBool();

            // Avoid duplicates
            const bool exists = std::any_of(m_devices.constBegin(), m_devices.constEnd(),
                [&address](const Model::BluetoothDevice &d) { return d.address == address; });
            if (exists) continue;

            Model::BluetoothDevice device(address, name, it.key(), true, connected);
            m_devices.append(device);
            changed = true;
            qInfo(lcBtHandler) << "paired device loaded name=" << name << " address=" << address;
        }

        if (changed)
            Q_EMIT pairedDeviceListChanged();
    }

    void LinuxBluetoothManager::subscribeToInterfacesAdded() {
        // BlueZ fires InterfacesAdded on / whenever a new object (including Device1) is
        // added to the object tree. This gives us the Name property as soon as BlueZ
        // creates the object — often before Qt's deviceUpdated signal fires.
        bool ok = QDBusConnection::systemBus().connect(
            "org.bluez", "/",
            "org.freedesktop.DBus.ObjectManager", "InterfacesAdded",
            this, SLOT(onBluezInterfacesAdded(QDBusObjectPath, BluezInterfaceList)));
        if (!ok)
            qWarning(lcBtHandler) << "failed to subscribe to BlueZ InterfacesAdded";
    }

    void LinuxBluetoothManager::onBluezInterfacesAdded(const QDBusObjectPath &path,
                                                   const BluezInterfaceList &interfaces) {
        if (!interfaces.contains("org.bluez.Device1")) return;

        const QVariantMap &props = interfaces["org.bluez.Device1"];
        const QString address = props.value("Address").toString();
        if (address.isEmpty()) return;

        const QString name = props.value("Name", props.value("Alias")).toString();
        if (name.isEmpty()) return;

        bool changed = false;
        for (auto &device : m_devices) {
            if (device.address == address && device.name != name) {
                qInfo(lcBtHandler) << "InterfacesAdded resolved name address=" << address << " name=" << name;
                device.name = name;
                changed = true;
                break;
            }
        }
        if (changed) {
            Q_EMIT unpairedDeviceListChanged();
            Q_EMIT pairedDeviceListChanged();
        }
        Q_UNUSED(path)
    }

    void LinuxBluetoothManager::refreshDeviceNamesFromBlueZ() {
        QDBusReply<BluezManagedObjects> reply = m_manager.call("GetManagedObjects");
        if (!reply.isValid()) {
            qWarning(lcBtHandler) << "refreshDeviceNamesFromBlueZ failed error=" << reply.error().message();
            return;
        }

        const BluezManagedObjects &objects = reply.value();
        bool changed = false;

        for (auto &device : m_devices) {
            if (!device.name.isEmpty()) continue; // already has a name

            // Look for this address in BlueZ's object tree
            for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
                const BluezInterfaceList &interfaces = it.value();
                if (!interfaces.contains("org.bluez.Device1")) continue;

                const QVariantMap &props = interfaces["org.bluez.Device1"];
                if (props.value("Address").toString() != device.address) continue;

                // Prefer Name (from inquiry/GATT), fall back to Alias (user-editable label)
                const QString name = props.value("Name", props.value("Alias")).toString();
                if (!name.isEmpty()) {
                    qInfo(lcBtHandler) << "name resolved from BlueZ address=" << device.address << " name=" << name;
                    device.name = name;
                    changed = true;
                }
                break;
            }
        }

        if (changed) {
            Q_EMIT unpairedDeviceListChanged();
            Q_EMIT pairedDeviceListChanged();
        }
    }
#endif

    void LinuxBluetoothManager::setActiveAdapter(const QString &address) {
        if (localDevice_->address().toString() == address) return;

        qInfo(lcBtHandler) << "adapter switch address=" << address;

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
                    this, &LinuxBluetoothManager::onPairingFinished);

            // Save preference
            configuration_->updateSettingByName(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress, address);
            configuration_->save();
        }
    }

    void LinuxBluetoothManager::enablePairingMode(bool enabled) {
        if (m_pairingModeEnabled == enabled) return;
        m_pairingModeEnabled = enabled;

        // Update host visibility so remote devices can find us
        if (localDevice_ && localDevice_->isValid()) {
            localDevice_->setHostMode(enabled
                ? QBluetoothLocalDevice::HostDiscoverable
                : QBluetoothLocalDevice::HostConnectable);
        }

#ifdef Q_OS_LINUX
        QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1",
                                    QDBusConnection::systemBus());
        if (agentManager.isValid()) {
            if (enabled) {
                agentManager.call("RegisterAgent",
                                  QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())),
                                  "DisplayYesNo");
                agentManager.call("RequestDefaultAgent",
                                  QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())));
                qInfo(lcBtHandler) << "pairing mode enabled — device is discoverable";
            } else {
                agentManager.call("UnregisterAgent",
                                  QVariant::fromValue(QDBusObjectPath(m_agent->objectPath())));
                qInfo(lcBtHandler) << "pairing mode disabled — device is connectable only";
            }
        } else {
            qWarning(lcBtHandler) << "agent manager not available";
        }
#endif
        Q_EMIT pairingModeEnabledChanged();
    }

    void LinuxBluetoothManager::disconnectDevice(const QString &address) {
        qInfo(lcBtHandler) << "disconnect device=" << address;

#ifdef Q_OS_LINUX
        QString pathStr;

        auto it = std::find_if(m_devices.begin(), m_devices.end(),
                               [&address](const Model::BluetoothDevice &d) { return d.address == address; });

        if (it != m_devices.end())
            pathStr = it->path.path();

        if (pathStr.isEmpty() || pathStr == "/") {
            QString cleanAddress = address;
            cleanAddress.replace(":", "_");
            pathStr = getBluezAdapterPath() + "/dev_" + cleanAddress;
        }

        qInfo(lcBtHandler) << "dbus disconnect path=" << pathStr;
        QDBusInterface deviceInterface("org.bluez", pathStr, "org.bluez.Device1", QDBusConnection::systemBus());
        deviceInterface.call(QDBus::NoBlock, "Disconnect");
#else
        qDebug(lcBtHandler) << "manual disconnect not supported on this platform";
#endif

        // Update local state regardless of platform
        auto it2 = std::find_if(m_devices.begin(), m_devices.end(),
                                [&address](const Model::BluetoothDevice &d) { return d.address == address; });
        if (it2 != m_devices.end())
            it2->connected = false;

        setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::BC_DISCONNECTED);
        Q_EMIT pairedDeviceListChanged();
    }
}
