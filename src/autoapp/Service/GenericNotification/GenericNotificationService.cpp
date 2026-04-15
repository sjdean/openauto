#include <f1x/openauto/autoapp/Service/GenericNotification/GenericNotificationService.hpp>
#include <fstream>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceNotify, "journeyos.service.notification")

namespace f1x::openauto::autoapp::service::genericnotification {

  GenericNotificationService::GenericNotificationService(aasdk::messenger::IMessenger::Pointer messenger)
      : channel_(std::make_shared<aasdk::channel::genericnotification::GenericNotificationService>(std::move(
            messenger))) {

  }

  void GenericNotificationService::start() {
      qDebug(lcServiceNotify) << "starting";
  }

  void GenericNotificationService::stop() {
      qDebug(lcServiceNotify) << "stopping";
  }

  void GenericNotificationService::pause() {
      qDebug(lcServiceNotify) << "pausing";
  }

  void GenericNotificationService::resume() {
      qDebug(lcServiceNotify) << "resuming";
  }

  void GenericNotificationService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceNotify) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *genericNotification = service->mutable_wifi_projection_service();
  }

  void GenericNotificationService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceNotify) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&GenericNotificationService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void GenericNotificationService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceNotify) << "channel error msg=" << e.what();
  }
}



