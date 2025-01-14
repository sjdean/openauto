#include <f1x/openauto/btservice/BluetoothHandler.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothServer.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x::openauto::btservice {
  BluetoothHandler::BluetoothHandler(btservice::IAndroidBluetoothService::Pointer androidBluetoothService,
                                     autoapp::configuration::IConfiguration::Pointer configuration)
  : configuration_(std::move(configuration)),
    androidBluetoothService_(std::move(androidBluetoothService)),
    androidBluetoothServer_(std::make_unique<btservice::AndroidBluetoothServer>(configuration_)) {

    OPENAUTO_LOG(info) << "[BluetoothHandler::BluetoothHandler] Starting Up...";

    QString adapterAddress = configuration_->getSettingByName<QString>("Bluetooth", "AdapterAddress");
    QBluetoothAddress address(adapterAddress);
    localDevice_ = std::make_unique<QBluetoothLocalDevice>(QBluetoothAddress());

    if (!localDevice_->isValid()) {
      OPENAUTO_LOG(error) << "[BluetoothHandler] Bluetooth adapter is not valid.";
    } else {
      OPENAUTO_LOG(info) << "[BluetoothHandler] Bluetooth adapter is valid.";
    }

    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString adapterPath = getAdapterPathByAddress(adapterAddress);
    if (!adapterPath.isEmpty()) {
      // Connect to BlueZ signals for device changes on the specific adapter
      QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RequestPinCode", this, SLOT(onRequestPinCode(QDBusObjectPath)));
      QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RequestConfirmation", this, SLOT(onRequestConfirmation(QDBusObjectPath, quint32)));
      QDBusConnection::systemBus().connect("org.bluez", adapterPath, "org.bluez.Adapter1", "DeviceFound", this, SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
      QDBusConnection::systemBus().connect("org.bluez", adapterPath, "org.bluez.Adapter1", "DeviceConnected", this, SLOT(onDeviceConnected(QDBusObjectPath)));
      QDBusConnection::systemBus().connect("org.bluez", adapterPath, "org.bluez.Adapter1", "DeviceDisconnected", this, SLOT(onDeviceDisconnected(QDBusObjectPath)));
    } else {
      qWarning() << "No adapter found with address:" << adapterAddress;
    }
    #else
      QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingDisplayPinCode, this, &BluetoothHandler::onPairingDisplayPinCode);
      QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingDisplayConfirmation, this, &BluetoothHandler::onPairingDisplayConfirmation);
      QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::error, this, &BluetoothHandler::onError);
    #endif

    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::pairingFinished, this, &BluetoothHandler::onPairingFinished);
    QObject::connect(localDevice_.get(), &QBluetoothLocalDevice::hostModeStateChanged, this, &BluetoothHandler::onHostModeStateChanged);

    // Turn Bluetooth on
    localDevice_->powerOn();

    // Make it visible to others
    localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    uint16_t portNumber = androidBluetoothServer_->start(address);

    if (portNumber == 0) {
      OPENAUTO_LOG(error) << "[BluetoothHandler::BluetoothHandler] Server start failed.";
      throw std::runtime_error("Unable to start bluetooth server");
    }

    OPENAUTO_LOG(info) << "[BluetoothHandler::BluetoothHandler] Listening for connections, address: " << address.toString().toStdString()
                       << ", port: " << portNumber;

   if (!androidBluetoothService_->registerService(portNumber, address)) {
      OPENAUTO_LOG(error) << "[BluetoothHandler::BluetoothHandler] Service registration failed.";
      throw std::runtime_error("Unable to register btservice");
    } else {
      OPENAUTO_LOG(info) << "[BluetoothHandler::BluetoothHandler] Service registered, port: " << portNumber;
    }

    // TODO: Connect to any previously paired devices
  }

  QString BluetoothHandler::getAdapterPathByAddress(const QString &adapterAddress) {

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
    return QString(); // No matching adapter found
  }

  void BluetoothHandler::shutdownService() {
    OPENAUTO_LOG(info) << "[BluetoothHandler::shutdownService] Shutdown initiated";
    androidBluetoothService_->unregisterService();
  }

  void BluetoothHandler::onPairingDisplayPinCode(const QBluetoothAddress &address, QString pin) {
    OPENAUTO_LOG(debug) << "[BluetoothHandler::onPairingDisplayPinCode] Pairing display PIN code: " << pin.toStdString();
  }

  void BluetoothHandler::onPairingDisplayConfirmation(const QBluetoothAddress &address, QString pin) {
    OPENAUTO_LOG(debug) << "[BluetoothHandler::onPairingDisplayConfirmation] Pairing display confirmation: " << pin.toStdString();

    // Here you can implement logic to show this PIN to the user or automatically accept if you trust all devices
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // TODO: Move to QDBus for monitoring BlueZ.
    #else
      localDevice_->pairingConfirmation(true); // Confirm pairing (for security, you might want to verify the PIN)
    #endif
  }

  void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing) {
    OPENAUTO_LOG(info) << "[BluetoothHandler::onPairingFinished] pairingFinished, address: " << address.toString().toStdString()
                       << ", pairing: " << pairing;
  }

  void BluetoothHandler::onError(QBluetoothLocalDevice::Error error) {
    OPENAUTO_LOG(warning) << "[BluetoothHandler::onError] Bluetooth error: " << error;
    // ... your logic to handle the error ...
  }

  void BluetoothHandler::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state) {
    OPENAUTO_LOG(info) << "[BluetoothHandler::onHostModeStateChanged] Host mode state changed: " << state;
    // ... your logic to handle the state change ...
  }
}