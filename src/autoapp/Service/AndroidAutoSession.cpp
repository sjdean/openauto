#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoSession.hpp>
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcSession, "journeyos.session")

namespace f1x::openauto::autoapp::service {

  AndroidAutoSession::AndroidAutoSession(boost::asio::io_service &ioService,
                                       aasdk::messenger::ICryptor::Pointer cryptor,
                                       aasdk::transport::ITransport::Pointer transport,
                                       aasdk::messenger::IMessenger::Pointer messenger,
                                       configuration::IConfiguration::Pointer configuration,
                                       ServiceList serviceList,
                                       IPinger::Pointer pinger,
                                       std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor)
      : strand_(ioService), cryptor_(std::move(cryptor)), transport_(std::move(transport)),
        messenger_(std::move(messenger)), controlServiceChannel_(
          std::make_shared<aasdk::channel::control::ControlServiceChannel>(strand_, messenger_)),
        configuration_(std::move(configuration)), serviceList_(std::move(serviceList)),
        pinger_(std::move(pinger)),
        eventHandler_(nullptr),
        androidAutoMonitor_(std::move(androidAutoMonitor)) {
  }

  AndroidAutoSession::~AndroidAutoSession() {
    qDebug(lcSession) << "destroy.";
  }

  void AndroidAutoSession::start(IAndroidAutoSessionEventHandler &eventHandler) {
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);

    strand_.dispatch([this, self = this->shared_from_this(), eventHandler = &eventHandler]() {
      qInfo(lcSession) << "start()";

      eventHandler_ = eventHandler;
      std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::start, std::placeholders::_1));

      auto versionRequestPromise = aasdk::channel::SendPromise::defer(strand_);
      versionRequestPromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                     std::placeholders::_1));

      qDebug(lcSession) << "Send Version Request.";
      controlServiceChannel_->sendVersionRequest(std::move(versionRequestPromise));
      controlServiceChannel_->receive(this->shared_from_this());
    });
  }

  void AndroidAutoSession::stop() {

    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);


    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcSession) << "stop()";

      try {
        eventHandler_ = nullptr;
        std::for_each(serviceList_.begin(), serviceList_.end(),
                      std::bind(&IService::stop, std::placeholders::_1));

        messenger_->stop();
        transport_->stop();
        cryptor_->deinit();
      } catch (...) {
        qCritical(lcSession) << "stop() - exception when stopping.";
      }
    });
  }

  void AndroidAutoSession::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcSession) << "pause()";

      try {
        std::for_each(serviceList_.begin(), serviceList_.end(),
                      std::bind(&IService::pause, std::placeholders::_1));
      } catch (...) {
        qCritical(lcSession) << "pause() - exception when pausing.";
      }
    });
  }

  void AndroidAutoSession::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcSession) << "resume()";

      try {
        std::for_each(serviceList_.begin(), serviceList_.end(),
                      std::bind(&IService::resume, std::placeholders::_1));
      } catch (...) {
        qCritical(lcSession) << "resume() exception when resuming.";
      }
    });
  }

  void AndroidAutoSession::onVersionResponse(uint16_t majorCode, uint16_t minorCode,
                                            aap_protobuf::shared::MessageStatus status) {
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);

    qInfo(lcSession) << "onVersionResponse()";
    qInfo(lcSession) << "Version Received: " << majorCode << "." << minorCode
                       << ", with status: " << status;

    if (status == aap_protobuf::shared::MessageStatus::STATUS_NO_COMPATIBLE_VERSION) {
      qCritical(lcSession) << "Version mismatch.";
      this->triggerQuit();
    } else {
      qDebug(lcSession) << "Version matches.";

      try {
        qInfo(lcSession) << "Beginning SSL handshake.";
        cryptor_->doHandshake();

        auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
        handshakePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                  std::placeholders::_1));
        controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
        controlServiceChannel_->receive(this->shared_from_this());
      }
      catch (const aasdk::error::Error &e) {
        qWarning(lcSession) << "Handshake Error.";
        this->onChannelError(e);
      }
    }
  }

  void AndroidAutoSession::onHandshake(const aasdk::common::DataConstBuffer &payload) {
    qInfo(lcSession) << "onHandshake()";
    qDebug(lcSession) << "Payload size: " << payload.size;

    try {
      cryptor_->writeHandshakeBuffer(payload);

      if (!cryptor_->doHandshake()) {
        qInfo(lcSession) << "Re-attempting handshake.";

        auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
        handshakePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                  std::placeholders::_1));
        controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
      } else {
        qInfo(lcSession) << "Handshake completed.";

        aap_protobuf::service::control::message::AuthResponse authCompleteIndication;
        authCompleteIndication.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

        auto authCompletePromise = aasdk::channel::SendPromise::defer(strand_);
        authCompletePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                     std::placeholders::_1));
        controlServiceChannel_->sendAuthComplete(authCompleteIndication, std::move(authCompletePromise));
      }

      controlServiceChannel_->receive(this->shared_from_this());
    }
    catch (const aasdk::error::Error &e) {
      qCritical(lcSession) << "Error during handshake";
      this->onChannelError(e);
    }
  }

  void AndroidAutoSession::onServiceDiscoveryRequest(
      const aap_protobuf::service::control::message::ServiceDiscoveryRequest &request) {
    qInfo(lcSession) << "onServiceDiscoveryRequest()";
    qDebug(lcSession) << "Type: " << request.label_text() << ", Model: "
                        << request.device_name();

    aap_protobuf::service::control::message::ServiceDiscoveryResponse serviceDiscoveryResponse;
    serviceDiscoveryResponse.mutable_channels()->Reserve(256);
    serviceDiscoveryResponse.set_driver_position(
        aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
    serviceDiscoveryResponse.set_display_name("CubeOne Journey");
    serviceDiscoveryResponse.set_probe_for_support(false);
    auto *connectionConfiguration = serviceDiscoveryResponse.mutable_connection_configuration();

    auto *pingConfiguration = connectionConfiguration->mutable_ping_configuration();
    pingConfiguration->set_tracked_ping_count(5);
    pingConfiguration->set_timeout_ms(3000);
    pingConfiguration->set_interval_ms(1000);
    pingConfiguration->set_high_latency_threshold_ms(200);


    auto *headUnitInfo = serviceDiscoveryResponse.mutable_headunit_info();

    serviceDiscoveryResponse.set_display_name("JourneyOS");
    headUnitInfo->set_make("CubeOne");
    headUnitInfo->set_model("Journey");
    headUnitInfo->set_year("2024");
    headUnitInfo->set_vehicle_id("2024110822150988");
    headUnitInfo->set_head_unit_make("CubeOne");
    headUnitInfo->set_head_unit_model("Journey");
    headUnitInfo->set_head_unit_software_build("1");
    headUnitInfo->set_head_unit_software_version("1.0");

    std::for_each(serviceList_.begin(), serviceList_.end(),
                  std::bind(&IService::fillFeatures, std::placeholders::_1, std::ref(serviceDiscoveryResponse)));

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendServiceDiscoveryResponse(serviceDiscoveryResponse, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onAudioFocusRequest(
      const aap_protobuf::service::control::message::AudioFocusRequest &request) {
    qInfo(lcSession) << "onAudioFocusRequest()";
    qDebug(lcSession) << "AudioFocusRequestType received: "
                        << AudioFocusRequestType_Name(request.audio_focus_type());

    /*
     * When the MD starts playing music for example, it sends a gain request. The HU replies:
     * STATE_GAIN - no restrictions
     * STATE_GAIN_MEDIA_ONLY when using a guidance channel
     * STATE_LOSS when vehicle is playing high priority sound after stopping native media (ie USB, RADIO)
     *
     * When HU starts playing music, we should send a STATE LOSS to stop MD music and guidance.
     */

    // If release, we should stop all playback
    // MD wants to play a sound, get a notifiation regarding GAIN
    // HU grants focus - to enable MD to send audio over both MEDIA and GUIDANCE channels.
    // MD can then play guidance over the MEDIA or GUIDANCE streams
    // HU should send STATE_LOSS to stop MD playing (ie if user starts radio player)
    aap_protobuf::service::control::message::AudioFocusStateType audioFocusStateType =
        request.audio_focus_type() ==
        aap_protobuf::service::control::message::AudioFocusRequestType::AUDIO_FOCUS_RELEASE
        ? aap_protobuf::service::control::message::AudioFocusStateType::AUDIO_FOCUS_STATE_LOSS
        : aap_protobuf::service::control::message::AudioFocusStateType::AUDIO_FOCUS_STATE_GAIN;

    qDebug(lcSession) << "AudioFocusStateType determined: "
                        << AudioFocusStateType_Name(audioFocusStateType);

    aap_protobuf::service::control::message::AudioFocusNotification response;
    response.set_focus_state(audioFocusStateType);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  [capture0 = this->shared_from_this()](auto &&PH1) {
                    capture0->onChannelError(std::forward<decltype(PH1)>(PH1));
                  });
    controlServiceChannel_->sendAudioFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onByeByeRequest(
      const aap_protobuf::service::control::message::ByeByeRequest &request) {
    qInfo(lcSession) << "onByeByeRequest()";
    qDebug(lcSession) << "Reason received: " << request.reason();

    aap_protobuf::service::control::message::ByeByeResponse response;
    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then(std::bind(&AndroidAutoSession::triggerQuit, this->shared_from_this()),
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));

    controlServiceChannel_->sendShutdownResponse(response, std::move(promise));
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);
  }

  void AndroidAutoSession::onByeByeResponse(
      const aap_protobuf::service::control::message::ByeByeResponse &response) {
    qInfo(lcSession) << "onByeByeResponse()";
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);
    this->triggerQuit();
  }

  void AndroidAutoSession::onNavigationFocusRequest(
      const aap_protobuf::service::control::message::NavFocusRequestNotification &request) {
    qInfo(lcSession) << "onNavigationFocusRequest()";
    qDebug(lcSession) << "NavFocusRequestNotification type received: "
                        << NavFocusType_Name(request.focus_type());

    /*
     * If the MD sends NAV_FOCUS_PROJECTED in the request, we should stop any local navigation on the HU and grant NAV_FOCUS_NATIVE in the response.
     * If the HU starts its own Nav, we should send NAV_FOCUS_NATIVE.
     *
     * For now, this is fine to be hardcoded as OpenAuto does not provide any local navigation, only that provided through Android Auto.
     */
    aap_protobuf::service::control::message::NavFocusNotification response;
    response.set_focus_type(
        aap_protobuf::service::control::message::NavFocusType::NAV_FOCUS_PROJECTED);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendNavigationFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onBatteryStatusNotification(
      const aap_protobuf::service::control::message::BatteryStatusNotification &notification) {
    qInfo(lcSession) << "onBatteryStatusNotification()";
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onPingRequest(const aap_protobuf::service::control::message::PingRequest &request) {
    qInfo(lcSession) << "onPingRequest()";
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onVoiceSessionRequest(
      const aap_protobuf::service::control::message::VoiceSessionNotification &request) {
    qInfo(lcSession) << "onVoiceSessionRequest()";
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onPingResponse(const aap_protobuf::service::control::message::PingResponse &response) {
    qInfo(lcSession) << "onPingResponse()";
    qDebug(lcSession) << "Timestamp: " << response.timestamp();
    pinger_->pong();
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcSession) << "onChannelError(): " << e.what();
    this->triggerQuit();
  }

  void AndroidAutoSession::triggerQuit() {
    qInfo(lcSession) << "triggerQuit()";
    if (eventHandler_ != nullptr) {
      eventHandler_->onAndroidAutoQuit();
    }
  }

  void AndroidAutoSession::schedulePing() {
    qInfo(lcSession) << "schedulePing()";
    auto promise = IPinger::Promise::defer(strand_);
    promise->then([this, self = this->shared_from_this()]() {
                    this->sendPing();
                    this->schedulePing();
                  },
                  [this, self = this->shared_from_this()](auto error) {
                    if (error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
                        error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                      qCritical(lcSession) << "Ping timer exceeded.";
                      this->triggerQuit();
                    }
                  });

    pinger_->ping(std::move(promise));
  }

  void AndroidAutoSession::sendPing() {
    qDebug(lcSession) << "sendPing()";
    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));

    aap_protobuf::service::control::message::PingRequest request;
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
    request.set_timestamp(timestamp.count());
    controlServiceChannel_->sendPingRequest(request, std::move(promise));
  }
}



