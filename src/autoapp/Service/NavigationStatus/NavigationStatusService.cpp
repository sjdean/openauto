#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/NavigationStatus/NavigationStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x::openauto::autoapp::service::navigationstatus {

  NavigationStatusService::NavigationStatusService(boost::asio::io_service &ioService,
                                                   aasdk::messenger::IMessenger::Pointer messenger)
      : strand_(ioService),
        timer_(ioService),
        channel_(std::make_shared<aasdk::channel::navigationstatus::NavigationStatusService>(strand_,
                                                                                             std::move(messenger))) {

  }

  void NavigationStatusService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] start()";
    });
  }

  void NavigationStatusService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] stop()";
    });
  }

  void NavigationStatusService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] pause()";
    });
  }

  void NavigationStatusService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] resume()";
    });
  }

  void NavigationStatusService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[NavigationStatusService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *navigationStatus = service->mutable_navigation_status_service();
  }

  void NavigationStatusService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[NavigationStatusService] onChannelOpenRequest()";
    OPENAUTO_LOG(info) << "[NavigationStatusService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&NavigationStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void NavigationStatusService::onStatusUpdate(
      const aap_protobuf::service::navigationstatus::message::NavigationStatus &navStatus) {
    channel_->receive(this->shared_from_this());
  }


  void NavigationStatusService::onTurnEvent(
      const aap_protobuf::service::navigationstatus::message::NavigationNextTurnEvent &turnEvent) {
    channel_->receive(this->shared_from_this());
  }

  void NavigationStatusService::onDistanceEvent(
      const aap_protobuf::service::navigationstatus::message::NavigationNextTurnDistanceEvent &distanceEvent) {
    channel_->receive(this->shared_from_this());
  }


  void NavigationStatusService::onChannelError(const aasdk::error::Error &e) {
    OPENAUTO_LOG(error) << "[NavigationStatusService] onChannelError(): " << e.what();
  }
}



