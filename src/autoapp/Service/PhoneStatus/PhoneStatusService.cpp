#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/PhoneStatus/PhoneStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace phonestatus {

          PhoneStatusService::PhoneStatusService(boost::asio::io_service &ioService,
                                                       aasdk::messenger::IMessenger::Pointer messenger)
              : strand_(ioService),
                timer_(ioService),
                channel_(std::make_shared<aasdk::channel::phonestatus::PhoneStatusService>(strand_, std::move(messenger))) {

          }

          void PhoneStatusService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] start()";
            });
          }

          void PhoneStatusService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] stop()";
            });
          }

          void PhoneStatusService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] pause()";
            });
          }

          void PhoneStatusService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] resume()";
            });
          }

          void PhoneStatusService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[PhoneStatusService] fillFeatures()";

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *phoneStatus = service->mutable_phone_status_service();
          }

          void PhoneStatusService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[PhoneStatusService] onChannelOpenRequest()";
            OPENAUTO_LOG(debug) << "[PhoneStatusService] Channel Id: " << request.service_id() << ", Priority: " << request.priority();

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
            OPENAUTO_LOG(error) << "[PhoneStatusService] onChannelError(): " << e.what();
          }
        }
      }
    }
  }
}