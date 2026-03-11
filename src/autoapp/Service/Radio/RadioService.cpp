#include <f1x/openauto/autoapp/Service/Radio/RadioService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceRadio, "journeyos.service.radio")

namespace f1x::openauto::autoapp::service::radio {

  RadioService::RadioService(boost::asio::io_service &ioService,
                             aasdk::messenger::IMessenger::Pointer messenger)
      : timer_(ioService),
        strand_(ioService),
        channel_(std::make_shared<aasdk::channel::radio::RadioService>(strand_, std::move(messenger))) {

  }

  void RadioService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "starting";
    });
  }

  void RadioService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "stopping";
    });
  }

  void RadioService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "pausing";
    });
  }

  void RadioService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "resuming";
    });
  }

  void RadioService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceRadio) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *radio = service->mutable_radio_service();
  }

  void RadioService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceRadio) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();


    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&RadioService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void RadioService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceRadio) << "channel error msg=" << e.what();
  }


}



