#include <QtBluetooth>
#include <QGuiApplication>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp>

// TODO: This isn't really Projection
namespace f1x::openauto::autoapp::projection {
  // TODO: Ensure we're passing in the adapter address correctly.
  // TODO: We may need to rename this to a LinuxBluetoothAdapter so we can abstract and create models for Windows and Mac
  LocalBluetoothDevice::LocalBluetoothDevice(const QString &adapterAddress, QObject *parent) : QObject(parent) {
    qRegisterMetaType<IBluetoothDevice::PairingPromise::Pointer>("PairingPromise::Pointer");

    this->moveToThread(QGuiApplication::instance()->thread());

    QMetaObject::invokeMethod(this, "createBluetoothLocalDevice", Qt::BlockingQueuedConnection,
                              Q_ARG(QString, adapterAddress));

  }

  void LocalBluetoothDevice::createBluetoothLocalDevice(const QString &adapterAddress) {
    OPENAUTO_LOG(info) << "[LocalBluetoothDevice] create.";

    QBluetoothAddress address(adapterAddress);
    localDevice_ = std::make_unique<QBluetoothLocalDevice>(address);

    // Turn Bluetooth on
    localDevice_->powerOn();

    // Make it visible to others
    localDevice_->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
  }

  void LocalBluetoothDevice::stop() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

  }

  bool LocalBluetoothDevice::isPaired(const std::string &address) const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return localDevice_->pairingStatus(QBluetoothAddress(QString::fromStdString(address))) !=
           QBluetoothLocalDevice::Unpaired;
  }

  std::string LocalBluetoothDevice::getAdapterAddress() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return localDevice_->isValid() ? localDevice_->address().toString().toStdString() : "";
  }

  bool LocalBluetoothDevice::isAvailable() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return localDevice_->isValid();
  }
}



