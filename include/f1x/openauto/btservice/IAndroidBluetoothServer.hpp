#pragma once
#include <memory>
#include <QBluetoothAddress>

namespace f1x::openauto::btservice {

  class IAndroidBluetoothServer {
  public:
    typedef std::shared_ptr<IAndroidBluetoothServer> Pointer;

    virtual ~IAndroidBluetoothServer() = default;

    virtual uint16_t start(const QBluetoothAddress &address) = 0;
  };

}


