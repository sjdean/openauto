#include <f1x/openauto/autoapp/Service/GenericNotification/GenericNotificationService.hpp>
#include <fstream>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceNotify, "journeyos.service.notification")

namespace f1x::openauto::autoapp::service::genericnotification {

  GenericNotificationService::GenericNotificationService(boost::asio::io_service &ioService,
                                                         aasdk::messenger::IMessenger::Pointer messenger)
      : strand_(ioService),
        timer_(ioService),
        channel_(std::make_shared<aasdk::channel::genericnotification::GenericNotificationService>(strand_, std::move(
            messenger))) {

  }

  void GenericNotificationService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceNotify) << "[GenericNotificationService] start()";
    });
  }

  void GenericNotificationService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceNotify) << "[GenericNotificationService] stop()";
    });
  }

  void GenericNotificationService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceNotify) << "[GenericNotificationService] pause()";
    });
  }

  void GenericNotificationService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceNotify) << "[GenericNotificationService] resume()";
    });
  }

  void GenericNotificationService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qInfo(lcServiceNotify) << "[GenericNotificationService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *genericNotification = service->mutable_wifi_projection_service();
  }

  void GenericNotificationService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceNotify) << "[GenericNotificationService] onChannelOpenRequest()";
    qDebug(lcServiceNotify) << "[GenericNotificationService] Channel Id: " << request.service_id() << ", Priority: "
                        << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&GenericNotificationService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void GenericNotificationService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceNotify) << "[GenericNotificationService] onChannelError(): " << e.what();
  }
}



