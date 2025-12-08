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
      qDebug(lcServiceRadio) << "[RadioService] start()";
    });
  }

  void RadioService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "[RadioService] stop()";
    });
  }

  void RadioService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "[RadioService] pause()";
    });
  }

  void RadioService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceRadio) << "[RadioService] resume()";
    });
  }

  void RadioService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qInfo(lcServiceRadio) << "[RadioService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *radio = service->mutable_radio_service();
  }

  void RadioService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceRadio) << "[RadioService] onChannelOpenRequest()";
    qDebug(lcServiceRadio) << "[RadioService] Channel Id: " << request.service_id() << ", Priority: "
                        << request.priority();


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
    qCritical(lcServiceRadio) << "[RadioService] onChannelError(): " << e.what();
  }


}



