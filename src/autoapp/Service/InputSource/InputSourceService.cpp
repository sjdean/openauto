#include <f1x/openauto/autoapp/Service/InputSource/InputSourceService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceInput, "journeyos.service.input")


namespace f1x::openauto::autoapp::service::inputsource {
  InputSourceService::InputSourceService(aasdk::messenger::IMessenger::Pointer messenger,
                                         projection::IInputDevice::Pointer inputDevice)
      : channel_(std::make_shared<aasdk::channel::inputsource::InputSourceService>(std::move(messenger))),
        inputDevice_(std::move(inputDevice)) {

  }

  void InputSourceService::start() {
      qInfo(lcServiceInput) << "starting";
  channel_->receive(this->shared_from_this());
  }

  void InputSourceService::stop() {
      qInfo(lcServiceInput) << "stopping";
  inputDevice_->stop();
  }

  void InputSourceService::pause() {
      qDebug(lcServiceInput) << "pausing";
  }

  void InputSourceService::resume() {
      qDebug(lcServiceInput) << "resuming";
  }

  void InputSourceService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceInput) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *inputChannel = service->mutable_input_source_service();

    const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

    for (const auto &buttonCode: supportedButtonCodes) {
      inputChannel->add_keycodes_supported(buttonCode);
    }

    if (inputDevice_->hasTouchscreen()) {
      const auto &touchscreenSurface = inputDevice_->getTouchscreenGeometry();
      auto touchscreenConfig = inputChannel->add_touchscreen();
      touchscreenConfig->set_type(aap_protobuf::service::inputsource::message::TouchScreenType::CAPACITIVE);
      touchscreenConfig->set_width(touchscreenSurface.width());
      touchscreenConfig->set_height(touchscreenSurface.height());
      qDebug(lcServiceInput) << "touchscreen configured width=" << touchscreenSurface.width();
    } else {
      qDebug(lcServiceInput) << "no touchscreen";
    }
  }

  void
  InputSourceService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceInput) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();


    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void InputSourceService::onKeyBindingRequest(
      const aap_protobuf::service::media::sink::message::KeyBindingRequest &request) {
    qDebug(lcServiceInput) << "key binding request count=" << request.keycodes_size();

    aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

    for (int i = 0; i < request.keycodes_size(); ++i) {
      if (std::find(supportedButtonCodes.begin(), supportedButtonCodes.end(), request.keycodes(i)) ==
          supportedButtonCodes.end()) {
        qWarning(lcServiceInput) << "unsupported key code=" << request.keycodes(i);
        status = aap_protobuf::shared::MessageStatus::STATUS_KEYCODE_NOT_BOUND;
        break;
      }
    }

    aap_protobuf::service::media::sink::message::KeyBindingResponse response;
    response.set_status(status);

    if (status == aap_protobuf::shared::MessageStatus::STATUS_SUCCESS) {
      inputDevice_->start(*this);
    }

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendKeyBindingResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void InputSourceService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceInput) << "channel error msg=" << e.what();
  }

  void InputSourceService::onButtonEvent(const projection::ButtonEvent &event) {
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());

          aap_protobuf::service::inputsource::message::InputReport inputReport;
      inputReport.set_timestamp(timestamp.count());

      if (event.code == aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER) {
        auto relativeEvent = inputReport.mutable_relative_event()->add_data();
        relativeEvent->set_delta(event.wheelDirection == projection::WheelDirection::LEFT ? -1 : 1);
        relativeEvent->set_keycode(event.code);
      } else {
        auto buttonEvent = inputReport.mutable_key_event()->add_keys();
        buttonEvent->set_metastate(0);
        buttonEvent->set_down(event.type == projection::ButtonEventType::PRESS);
        buttonEvent->set_longpress(false);
        buttonEvent->set_keycode(event.code);
      }

      auto promise = aasdk::channel::SendPromise::defer();
      promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                       std::placeholders::_1));
      channel_->sendInputReport(inputReport, std::move(promise));
    
  }

  void InputSourceService::onTouchEvent(const projection::TouchEvent &event) {
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());

          aap_protobuf::service::inputsource::message::InputReport inputReport;
      inputReport.set_timestamp(timestamp.count());

      auto touchEvent = inputReport.mutable_touch_event();
      touchEvent->set_action(event.type);
      auto touchLocation = touchEvent->add_pointer_data();
      touchLocation->set_x(event.x);
      touchLocation->set_y(event.y);
      touchLocation->set_pointer_id(0);

      auto promise = aasdk::channel::SendPromise::defer();
      promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                       std::placeholders::_1));
      channel_->sendInputReport(inputReport, std::move(promise));
    
  }
}



