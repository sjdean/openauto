#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/MediaPlaybackStatus/MediaPlaybackStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediaplaybackstatus {

          MediaPlaybackStatusService::MediaPlaybackStatusService(boost::asio::io_service &ioService,
                                                       aasdk::messenger::IMessenger::Pointer messenger)
              : strand_(ioService),
                timer_(ioService),
                channel_(std::make_shared<aasdk::channel::mediaplaybackstatus::MediaPlaybackStatusService>(strand_, std::move(messenger))) {

          }

          void MediaPlaybackStatusService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] start()";
            });
          }

          void MediaPlaybackStatusService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] stop()";
            });
          }

          void MediaPlaybackStatusService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] pause()";
            });
          }

          void MediaPlaybackStatusService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] resume()";
            });
          }

          void MediaPlaybackStatusService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] fillFeatures()";

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *mediaPlaybackStatus = service->mutable_media_playback_service();
          }

          void MediaPlaybackStatusService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] onChannelOpenRequest()";
            OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] Channel Id: " << request.service_id() << ", Priority: " << request.priority();

            aap_protobuf::service::control::message::ChannelOpenResponse response;
            const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&MediaPlaybackStatusService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));

            channel_->receive(this->shared_from_this());
          }


          void MediaPlaybackStatusService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[MediaPlaybackStatusService] onChannelError(): " << e.what();
          }
        }
      }
    }
  }
}