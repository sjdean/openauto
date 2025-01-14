#include <QtBluetooth>
#include <QGuiApplication>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp>

// TODO: Check Overlap with BluetoothHandler
namespace f1x::openauto::autoapp::projection {

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

    // Pairing signals are being handled by btservice

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



