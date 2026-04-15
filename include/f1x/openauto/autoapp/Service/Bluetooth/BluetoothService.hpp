#pragma once
#include <aasdk/Channel/Bluetooth/BluetoothService.hpp>
#include <f1x/openauto/autoapp/Projection/IBluetoothDevice.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x::openauto::autoapp::service::bluetooth {

  class BluetoothService
      : public aasdk::channel::bluetooth::IBluetoothServiceEventHandler,
        public IService,
        public std::enable_shared_from_this<BluetoothService> {
  public:
    BluetoothService(aasdk::messenger::IMessenger::Pointer messenger,
                     projection::IBluetoothDevice::Pointer bluetoothDevice);

    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

    void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

    void onBluetoothPairingRequest(
      const aap_protobuf::service::bluetooth::message::BluetoothPairingRequest &request) override;

    void onBluetoothAuthenticationResult(const aap_protobuf::service::bluetooth::message::BluetoothAuthenticationResult &request) override;

    void onChannelError(const aasdk::error::Error &e) override;


  private:
    using std::enable_shared_from_this<BluetoothService>::shared_from_this;

    void sendBluetoothAuthenticationData();
    aasdk::channel::bluetooth::BluetoothService::Pointer channel_;
    projection::IBluetoothDevice::Pointer bluetoothDevice_;
  };

}
