

#pragma once#include <aasdk/Channel/Bluetooth/BluetoothService.hpp>
#include <f1x/openauto/autoapp/Projection/IBluetoothDevice.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x::openauto::autoapp::service {

  class Service
      : public IService {
  public:
    Service(boost::asio::io_service &ioService);

    void start() override;

    void stop() override;

    void pause() override;

    void resume() override;

    void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

  private:

    boost::asio::io_service::strand strand_;

  };
}
