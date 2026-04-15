#include <f1x/openauto/autoapp/Service/VendorExtension/VendorExtensionService.hpp>
#include <fstream>
#include <QString>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceVendor, "journeyos.service.vendor")

namespace f1x::openauto::autoapp::service::vendorextension {

  VendorExtensionService::VendorExtensionService(aasdk::messenger::IMessenger::Pointer messenger)
    : channel_(
        std::make_shared<aasdk::channel::vendorextension::VendorExtensionService>(std::move(messenger))) {

  }

  void VendorExtensionService::start() {
      qDebug(lcServiceVendor) << "starting";
  }

  void VendorExtensionService::stop() {
      qDebug(lcServiceVendor) << "stopping";
  }

  void VendorExtensionService::pause() {
      qDebug(lcServiceVendor) << "pausing";
  }

  void VendorExtensionService::resume() {
      qDebug(lcServiceVendor) << "resuming";
  }

  void VendorExtensionService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceVendor) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *vendorExtension = service->mutable_vendor_extension_service();
  }

  void VendorExtensionService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceVendor) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&VendorExtensionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void VendorExtensionService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceVendor) << "channel error msg=" << e.what();
  }
}



