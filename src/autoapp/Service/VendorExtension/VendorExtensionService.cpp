#include <f1x/openauto/autoapp/Service/VendorExtension/VendorExtensionService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceVendor, "journeyos.service.vendor")

namespace f1x::openauto::autoapp::service::vendorextension {

  VendorExtensionService::VendorExtensionService(boost::asio::io_service &ioService,
                                                 aasdk::messenger::IMessenger::Pointer messenger)
    : timer_(ioService),
      strand_(ioService),
      channel_(
        std::make_shared<aasdk::channel::vendorextension::VendorExtensionService>(strand_, std::move(messenger))) {

  }

  void VendorExtensionService::start() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceVendor) << "[VendorExtensionService] start()";
    });
  }

  void VendorExtensionService::stop() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceVendor) << "[VendorExtensionService] stop()";
    });
  }

  void VendorExtensionService::pause() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceVendor) << "[VendorExtensionService] pause()";
    });
  }

  void VendorExtensionService::resume() {
    strand_.dispatch([self = this->shared_from_this()]() {
      qInfo(lcServiceVendor) << "[VendorExtensionService] resume()";
    });
  }

  void VendorExtensionService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qInfo(lcServiceVendor) << "[VendorExtensionService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *vendorExtension = service->mutable_vendor_extension_service();
  }

  void VendorExtensionService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceVendor) << "[VendorExtensionService] onChannelOpenRequest()";
    qInfo(lcServiceVendor) << "[VendorExtensionService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&VendorExtensionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void VendorExtensionService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceVendor) << "[VendorExtensionService] onChannelError(): " << e.what();
  }
}



