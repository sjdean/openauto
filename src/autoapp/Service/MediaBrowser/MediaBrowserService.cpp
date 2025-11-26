#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/MediaBrowser/MediaBrowserService.hpp>
#include <fstream>
#include <QString>

namespace f1x::openauto::autoapp::service::mediabrowser {

  MediaBrowserService::MediaBrowserService(boost::asio::io_service &ioService,
                                           aasdk::messenger::IMessenger::Pointer messenger)
      : strand_(ioService),
        timer_(ioService),
        channel_(std::make_shared<aasdk::channel::mediabrowser::MediaBrowserService>(strand_, std::move(messenger))) {

  }

  void MediaBrowserService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaBrowserService] start()";
    });
  }

  void MediaBrowserService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaBrowserService] stop()";
    });
  }

  void MediaBrowserService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaBrowserService] pause()";
    });
  }

  void MediaBrowserService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaBrowserService] resume()";
    });
  }

  void MediaBrowserService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[MediaBrowserService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *mediaBrowser = service->mutable_media_browser_service();
  }

  void MediaBrowserService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[MediaBrowserService] onChannelOpenRequest()";
    OPENAUTO_LOG(info) << "[MediaBrowserService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

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
    OPENAUTO_LOG(error) << "[MediaBrowserService] onChannelError(): " << e.what();
  }
}



