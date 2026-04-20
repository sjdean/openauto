#include <f1x/openauto/autoapp/Service/PhoneStatus/PhoneStatusService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServicePhoneStatus, "journeyos.service.phonestatus")

namespace f1x::openauto::autoapp::service::phonestatus {

  PhoneStatusService::PhoneStatusService(aasdk::messenger::IMessenger::Pointer messenger)
      : channel_(std::make_shared<aasdk::channel::phonestatus::PhoneStatusService>(std::move(messenger))) {

  }

  // TODO(phone-status — 5.1): This service is a stub. Full implementation requires:
  //
  // 1. aasdk change — add onPhoneStatus() callback to IPhoneStatusServiceEventHandler:
  //      virtual void onPhoneStatus(
  //          const aap_protobuf::service::phonestatus::message::PhoneStatus&) = 0;
  //    The channel already receives messages; they currently have nowhere to go.
  //
  // 2. Call channel_->receive(shared_from_this()) in start() so messages are actually read.
  //
  // 3. Implement onPhoneStatus() here to forward call state to AndroidAutoMonitor
  //    (or a dedicated PhoneStatusMonitor). PhoneStatus.Call.State carries:
  //      IN_CALL, INCOMING, ON_HOLD, CONFERENCED, MUTED, INACTIVE
  //    plus caller_number, caller_id, caller_thumbnail, call_duration_seconds.
  //
  // 4. Expose call state as a Q_PROPERTY on AndroidAutoMonitor (or new PhoneStatusMonitor)
  //    for QML — used to show a call overlay with caller ID, duration, and mute button.
  //
  // Note: PhoneStatus is distinct from AudioFocusRequest. Audio focus tells the HU
  // *when* to duck its media. PhoneStatus tells the HU *what is happening* on the phone
  // (call state, caller identity). Both are needed for a complete call UX.

  void PhoneStatusService::start() {
      qDebug(lcServicePhoneStatus) << "starting (stub — see TODO above)";
  }

  void PhoneStatusService::stop() {
      qDebug(lcServicePhoneStatus) << "stopping";
  }

  void PhoneStatusService::pause() {
      qDebug(lcServicePhoneStatus) << "pausing";
  }

  void PhoneStatusService::resume() {
      qDebug(lcServicePhoneStatus) << "resuming";
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

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&PhoneStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }


  void PhoneStatusService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServicePhoneStatus) << "channel error msg=" << e.what();
  }
}



