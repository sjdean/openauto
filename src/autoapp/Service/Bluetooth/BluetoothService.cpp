#include <f1x/openauto/autoapp/Service/Bluetooth/BluetoothService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceBt, "journeyos.service.bluetooth")

namespace f1x::openauto::autoapp::service::bluetooth {

  /// AndroidAuto channel to advise bluetooth availability and to manage HFP/A2DP pairing requests.
  BluetoothService::BluetoothService(aasdk::messenger::IMessenger::Pointer messenger,
                                     projection::IBluetoothDevice::Pointer bluetoothDevice)
      : channel_(std::make_shared<aasdk::channel::bluetooth::BluetoothService>(std::move(messenger))),
        bluetoothDevice_(std::move(bluetoothDevice)) {

  }

  void BluetoothService::start() {
      qInfo(lcServiceBt) << "starting";
  channel_->receive(this->shared_from_this());
  }

  void BluetoothService::stop() {
      qInfo(lcServiceBt) << "stopping";
  bluetoothDevice_->stop();
  }

  void BluetoothService::pause() {
      qInfo(lcServiceBt) << "paused";
  }

  void BluetoothService::resume() {
      qInfo(lcServiceBt) << "resumed";
  }

  void BluetoothService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto bluetooth = service->mutable_bluetooth_service();

    if (bluetoothDevice_->isAvailable()) {
      qInfo(lcServiceBt) << "adapter address=" << bluetoothDevice_->getAdapterAddress();

      // If the HU wants the MD to skip the Bluetooth Pairing and Connection process, the HU can declare its address as SKIP_THIS_BLUETOOTH
      bluetooth->set_car_address(bluetoothDevice_->getAdapterAddress());

      // AAP supports both PIN and Numeric Comparison as pairing methods.
      bluetooth->add_supported_pairing_methods(
          aap_protobuf::service::bluetooth::message::BluetoothPairingMethod::BLUETOOTH_PAIRING_PIN);
      bluetooth->add_supported_pairing_methods(
          aap_protobuf::service::bluetooth::message::BluetoothPairingMethod::BLUETOOTH_PAIRING_NUMERIC_COMPARISON);
    } else {
      qWarning(lcServiceBt) << "bluetooth not available";
      bluetooth->set_car_address("");
      bluetooth->add_supported_pairing_methods(
          aap_protobuf::service::bluetooth::message::BluetoothPairingMethod::BLUETOOTH_PAIRING_UNAVAILABLE);
    }
  }

  void BluetoothService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceBt) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&BluetoothService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void BluetoothService::onBluetoothPairingRequest(
      const aap_protobuf::service::bluetooth::message::BluetoothPairingRequest &request) {
    const auto isPaired = bluetoothDevice_->isPaired(request.phone_address());
    qInfo(lcServiceBt) << "pairing request phone=" << request.phone_address() << " already_paired=" << isPaired;

    aap_protobuf::service::bluetooth::message::BluetoothPairingResponse response;

    /*
     * The HU must always send a STATUS_SUCCESS response, or STATUS_BLUETOOTH_PAIRING_DELAYED if:
     *    there's a delay in allowing bluetooth
     *    the HU is already engaged in a bluetooth call
     */
    // TODO: Consider when we need to send STATUS_BLUETOOTH_PAIRING_DELAYED
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);
    response.set_already_paired(isPaired);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then(std::bind(&BluetoothService::sendBluetoothAuthenticationData, this->shared_from_this()),
                  std::bind(&BluetoothService::onChannelError, this->shared_from_this(),
                            std::placeholders::_1));
    channel_->sendBluetoothPairingResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void BluetoothService::sendBluetoothAuthenticationData() {
    qInfo(lcServiceBt) << "sending authentication data";

    aap_protobuf::service::bluetooth::message::BluetoothAuthenticationData data;
    // TODO: Do we need to generate a random pin, or is 123456 sufficient?
    data.set_auth_data("123456");
    data.set_pairing_method(
        aap_protobuf::service::bluetooth::message::BluetoothPairingMethod::BLUETOOTH_PAIRING_PIN);
    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&BluetoothService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendBluetoothAuthenticationData(data, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void BluetoothService::onBluetoothAuthenticationResult(
      const aap_protobuf::service::bluetooth::message::BluetoothAuthenticationResult &request) {
    qInfo(lcServiceBt) << "auth result status=" << request.status();
    aap_protobuf::service::bluetooth::message::BluetoothPairingResponse response;
    channel_->receive(this->shared_from_this());
  }

  void BluetoothService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceBt) << "channel error=" << e.what();
  }

}
