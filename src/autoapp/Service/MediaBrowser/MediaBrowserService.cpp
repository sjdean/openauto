#include <f1x/openauto/autoapp/Service/MediaBrowser/MediaBrowserService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceBrowser, "journeyos.service.mediabrowser")


namespace f1x::openauto::autoapp::service::mediabrowser {

  MediaBrowserService::MediaBrowserService(aasdk::messenger::IMessenger::Pointer messenger)
      : channel_(std::make_shared<aasdk::channel::mediabrowser::MediaBrowserService>(std::move(messenger))) {

  }

  void MediaBrowserService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceBrowser) << "starting";
    });
  }

  void MediaBrowserService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceBrowser) << "stopping";
    });
  }

  void MediaBrowserService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceBrowser) << "pausing";
    });
  }

  void MediaBrowserService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qDebug(lcServiceBrowser) << "resuming";
    });
  }

  void MediaBrowserService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceBrowser) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *mediaBrowser = service->mutable_media_browser_service();
  }

  void MediaBrowserService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceBrowser) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&MediaBrowserService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void MediaBrowserService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceBrowser) << "channel error msg=" << e.what();
  }
}



