#include <f1x/openauto/autoapp/Service/PhoneStatus/PhoneStatusService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServicePhoneStatus, "journeyos.service.phonestatus")

namespace f1x::openauto::autoapp::service::phonestatus {

  PhoneStatusService::PhoneStatusService(boost::asio::io_service &ioService,
                                         aasdk::messenger::IMessenger::Pointer messenger)
      : timer_(ioService),
        strand_(ioService),
        channel_(std::make_shared<aasdk::channel::phonestatus::PhoneStatusService>(strand_, std::move(messenger))) {

  }

  void PhoneStatusService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServicePhoneStatus) << "starting";
    });
  }

  void PhoneStatusService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServicePhoneStatus) << "stopping";
    });
  }

  void PhoneStatusService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServicePhoneStatus) << "pausing";
    });
  }

  void PhoneStatusService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServicePhoneStatus) << "resuming";
    });
  }

  void PhoneStatusService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServicePhoneStatus) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *phoneStatus = service->mutable_phone_status_service();
  }

  void
  PhoneStatusService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServicePhoneStatus) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&PhoneStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }


  void PhoneStatusService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServicePhoneStatus) << "channel error msg=" << e.what();
  }
}



