#pragma once

#include <f1x/openauto/autoapp/Projection/IBluetoothDevice.hpp>

namespace f1x::openauto::autoapp::projection {

  class DummyBluetoothDevice : public IBluetoothDevice {
  public:
    void stop() override;

    bool isPaired(const std::string &address) const override;

    std::string getAdapterAddress() const override;

    bool isAvailable() const override;
  };

}



