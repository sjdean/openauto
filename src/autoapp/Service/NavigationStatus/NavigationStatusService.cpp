#include <f1x/openauto/autoapp/Service/NavigationStatus/NavigationStatusService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceNavigation, "journeyos.service.navigation")

namespace f1x::openauto::autoapp::service::navigationstatus {

  NavigationStatusService::NavigationStatusService(boost::asio::io_service &ioService,
                                                   aasdk::messenger::IMessenger::Pointer messenger)
    : timer_(ioService),
      strand_(ioService),
      channel_(std::make_shared<aasdk::channel::navigationstatus::NavigationStatusService>(strand_,
        std::move(messenger))) {

  }

  void NavigationStatusService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceNavigation) << "starting";
    });
  }

  void NavigationStatusService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceNavigation) << "stopping";
    });
  }

  void NavigationStatusService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceNavigation) << "pausing";
    });
  }

  void NavigationStatusService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceNavigation) << "resuming";
    });
  }

  void NavigationStatusService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceNavigation) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *navigationStatus = service->mutable_navigation_status_service();
  }

  void NavigationStatusService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceNavigation) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&NavigationStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void NavigationStatusService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceNavigation) << "channel error msg=" << e.what();
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
}



