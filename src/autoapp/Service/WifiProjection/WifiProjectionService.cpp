#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/WifiProjection/WifiProjectionService.hpp>
#include <fstream>
#include <QString>
#include <QNetworkInterface>
#include <utility>

namespace f1x::openauto::autoapp::service::wifiprojection {
  WifiProjectionService::WifiProjectionService(boost::asio::io_service &ioService,
                                               aasdk::messenger::IMessenger::Pointer messenger,
                                               configuration::IConfiguration::Pointer configuration)
      : configuration_(std::move(configuration)),
        timer_(ioService),
        strand_(ioService),
        channel_(
            std::make_shared<aasdk::channel::wifiprojection::WifiProjectionService>(strand_, std::move(messenger))) {

  }

  void WifiProjectionService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] start()";
    });
  }

  void WifiProjectionService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] stop()";
    });
  }

  void WifiProjectionService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] pause()";
    });
  }

  void WifiProjectionService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] resume()";
    });
  }

  void WifiProjectionService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[WifiProjectionService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *wifiChannel = service->mutable_wifi_projection_service();
    wifiChannel->set_car_wifi_bssid(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
  }

  void WifiProjectionService::onWifiCredentialsRequest(
      const aap_protobuf::service::wifiprojection::message::WifiCredentialsRequest &request) {

    OPENAUTO_LOG(info) << "[WifiProjectionService] onWifiCredentialsRequest()";

    aap_protobuf::service::wifiprojection::message::WifiCredentialsResponse response;

    response.set_access_point_type(aap_protobuf::service::wifiprojection::message::AccessPointType::STATIC);
    response.set_car_wifi_ssid(configuration_->getSettingByName<QString>("WiFi", "SSID").toStdString());
    response.set_car_wifi_password(configuration_->getSettingByName<QString>("WiFi", "Password").toStdString());
    response.set_car_wifi_security_mode(
        aap_protobuf::service::wifiprojection::message::WifiSecurityMode::WPA2_PERSONAL);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiProjectionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendWifiCredentialsResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());

  }

  void WifiProjectionService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[WifiProjectionService] onChannelOpenRequest()";
    OPENAUTO_LOG(debug) << "[WifiProjectionService] Channel Id: " << request.service_id() << ", Priority: "
                        << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiProjectionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void WifiProjectionService::onChannelError(const aasdk::error::Error &e) {
    OPENAUTO_LOG(error) << "[WifiProjectionService] onChannelError(): " << e.what();
  }


}



