#include <f1x/openauto/autoapp/Projection/DummyBluetoothDevice.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBtDummy, "journeyos.bluetooth.dummy")

namespace f1x::openauto::autoapp::projection {

  void DummyBluetoothDevice::stop() {

  }

  bool DummyBluetoothDevice::isPaired(const std::string &) const {
    return false;
  }

  std::string DummyBluetoothDevice::getAdapterAddress() const {
    return "";
  }

  bool DummyBluetoothDevice::isAvailable() const {
    return false;
  }

}



