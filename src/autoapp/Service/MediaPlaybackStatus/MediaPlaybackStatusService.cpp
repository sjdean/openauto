#include <f1x/openauto/autoapp/Service/MediaPlaybackStatus/MediaPlaybackStatusService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServicePlayback, "journeyos.service.mediaplayback")


namespace f1x::openauto::autoapp::service::mediaplaybackstatus {

  MediaPlaybackStatusService::MediaPlaybackStatusService(aasdk::messenger::IMessenger::Pointer messenger)
      : channel_(std::make_shared<aasdk::channel::mediaplaybackstatus::MediaPlaybackStatusService>(std::move(
          messenger))) {

  }

  void MediaPlaybackStatusService::start() {
      qDebug(lcServicePlayback) << "starting";
  }

  void MediaPlaybackStatusService::stop() {
      qDebug(lcServicePlayback) << "stopping";
  }

  void MediaPlaybackStatusService::pause() {
      qDebug(lcServicePlayback) << "pausing";
  }

  void MediaPlaybackStatusService::resume() {
      qDebug(lcServicePlayback) << "resuming";
  }

  void MediaPlaybackStatusService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServicePlayback) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *mediaPlaybackStatus = service->mutable_media_playback_service();
  }

  void MediaPlaybackStatusService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServicePlayback) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&MediaPlaybackStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }


  void MediaPlaybackStatusService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServicePlayback) << "channel error msg=" << e.what();
  }
}



