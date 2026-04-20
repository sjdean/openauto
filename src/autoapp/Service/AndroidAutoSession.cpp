#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoSession.hpp>
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoAudioFocusState.hpp"
#include <qloggingcategory.h>
#include <QtEndian>
Q_LOGGING_CATEGORY(lcSession, "journeyos.session")

namespace f1x::openauto::autoapp::service {

  AndroidAutoSession::AndroidAutoSession(aasdk::messenger::ICryptor::Pointer cryptor,
                                       aasdk::transport::ITransport::Pointer transport,
                                       aasdk::messenger::IMessenger::Pointer messenger,
                                       configuration::IConfiguration::Pointer configuration,
                                       ServiceList serviceList,
                                       IPinger::Pointer pinger,
                                       std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor)
      : cryptor_(std::move(cryptor)), transport_(std::move(transport)),
        messenger_(std::move(messenger)), controlServiceChannel_(
          std::make_shared<aasdk::channel::control::ControlServiceChannel>(messenger_)),
        configuration_(std::move(configuration)), serviceList_(std::move(serviceList)),
        pinger_(std::move(pinger)),
        eventHandler_(nullptr),
        androidAutoMonitor_(std::move(androidAutoMonitor)) {
  }

  AndroidAutoSession::~AndroidAutoSession() {
    qInfo(lcSession) << "session destroyed";
  }

  void AndroidAutoSession::start(IAndroidAutoSessionEventHandler &eventHandler) {
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);

      qInfo(lcSession) << ">>> session starting";

  eventHandler_ = eventHandler;
  std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::start, std::placeholders::_1));

  auto versionRequestPromise = aasdk::channel::SendPromise::defer();
  versionRequestPromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                 std::placeholders::_1));

  qDebug(lcSession) << "sending version request";
  controlServiceChannel_->sendVersionRequest(std::move(versionRequestPromise));
  controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::stop() {

    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);


      qInfo(lcSession) << ">>> session stopping";

  try {
    eventHandler_ = nullptr;
    std::for_each(serviceList_.begin(), serviceList_.end(),
                  std::bind(&IService::stop, std::placeholders::_1));

    messenger_->stop();
    transport_->stop();
    cryptor_->deinit();
  } catch (...) {
    qCritical(lcSession) << "exception while stopping session";
  }
  }

  void AndroidAutoSession::pause() {
      qInfo(lcSession) << ">>> session pausing";

  try {
    std::for_each(serviceList_.begin(), serviceList_.end(),
                  std::bind(&IService::pause, std::placeholders::_1));
  } catch (...) {
    qCritical(lcSession) << "exception while pausing session";
  }
  }

  void AndroidAutoSession::resume() {
      qInfo(lcSession) << ">>> session resuming";

  try {
    std::for_each(serviceList_.begin(), serviceList_.end(),
                  std::bind(&IService::resume, std::placeholders::_1));
  } catch (...) {
    qCritical(lcSession) << "exception while resuming session";
  }
  }

  void AndroidAutoSession::onVersionResponse(uint16_t majorCode, uint16_t minorCode,
                                            aap_protobuf::shared::MessageStatus status) {
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);

    qInfo(lcSession) << "protocol version="
                     << qFromBigEndian<quint16>(majorCode) << "."
                     << qFromBigEndian<quint16>(minorCode)
                     << "status=" << status;

    if (status == aap_protobuf::shared::MessageStatus::STATUS_NO_COMPATIBLE_VERSION) {
      qCritical(lcSession) << "incompatible protocol version";
      this->triggerQuit();
    } else {
      try {
        qInfo(lcSession) << "SSL handshake starting";
        cryptor_->doHandshake();

        auto handshakePromise = aasdk::channel::SendPromise::defer();
        handshakePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                  std::placeholders::_1));
        controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
        controlServiceChannel_->receive(this->shared_from_this());
      }
      catch (const aasdk::error::Error &e) {
        qWarning(lcSession) << "SSL handshake failed";
        this->onChannelError(e);
      }
    }
  }

  void AndroidAutoSession::onHandshake(const aasdk::common::DataConstBuffer &payload) {
    qDebug(lcSession) << "handshake payload bytes=" << payload.size;

    try {
      cryptor_->writeHandshakeBuffer(payload);

      if (!cryptor_->doHandshake()) {
        qInfo(lcSession) << "SSL handshake continuing";

        auto handshakePromise = aasdk::channel::SendPromise::defer();
        handshakePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                  std::placeholders::_1));
        controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
      } else {
        qInfo(lcSession) << "SSL handshake completed";

        aap_protobuf::service::control::message::AuthResponse authCompleteIndication;
        authCompleteIndication.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

        auto authCompletePromise = aasdk::channel::SendPromise::defer();
        authCompletePromise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                                     std::placeholders::_1));
        controlServiceChannel_->sendAuthComplete(authCompleteIndication, std::move(authCompletePromise));
      }

      controlServiceChannel_->receive(this->shared_from_this());
    }
    catch (const aasdk::error::Error &e) {
      qCritical(lcSession) << "handshake error";
      this->onChannelError(e);
    }
  }

  void AndroidAutoSession::onServiceDiscoveryRequest(
      const aap_protobuf::service::control::message::ServiceDiscoveryRequest &request) {
    qInfo(lcSession) << "service discovery label=" << request.label_text() << "model=" << request.device_name();

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

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));

    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_CONNECTED);

    controlServiceChannel_->sendServiceDiscoveryResponse(serviceDiscoveryResponse, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onAudioFocusRequest(
      const aap_protobuf::service::control::message::AudioFocusRequest &request) {
    using ReqType   = aap_protobuf::service::control::message::AudioFocusRequestType;
    using StateType = aap_protobuf::service::control::message::AudioFocusStateType;
    using FocusState = common::Enum::AndroidAutoAudioFocusState;

    qDebug(lcSession) << "audio focus request=" << AudioFocusRequestType_Name(request.audio_focus_type());

    // Map the AA focus request to our monitor state and the response we send back.
    //
    // AA sends focus requests when it wants to play audio; the HU grants or denies
    // and signals its own media players to pause/duck/resume accordingly.
    //
    // Response meanings:
    //   GAIN             — HU grants exclusive focus, no restrictions.
    //   GAIN_TRANSIENT   — HU grants temporary focus; will reclaim shortly.
    //   LOSS             — HU withdraws focus (used for RELEASE).
    //
    // TODO(audio-focus — 5.1): When HU media players exist, respond with
    //   GAIN_MEDIA_ONLY when HU is playing guidance of its own (native nav).
    //   Send AUDIO_FOCUS_STATE_LOSS proactively to AA when HU starts playing media,
    //   so AA pauses its own media stream while the HU source has priority.

    StateType responseState;
    FocusState::Value monitorState;

    switch (request.audio_focus_type()) {
      case ReqType::AUDIO_FOCUS_GAIN:
        // AA wants exclusive audio — stop any HU media.
        monitorState  = FocusState::Gained;
        responseState = StateType::AUDIO_FOCUS_STATE_GAIN;
        break;

      case ReqType::AUDIO_FOCUS_GAIN_TRANSIENT:
        // AA wants brief exclusive audio — pause HU media, expect resume soon.
        monitorState  = FocusState::GainedTransient;
        responseState = StateType::AUDIO_FOCUS_STATE_GAIN_TRANSIENT;
        break;

      case ReqType::AUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK:
        // AA wants audio but HU may duck instead of stopping (e.g. navigation guidance).
        monitorState  = FocusState::GainedCanDuck;
        responseState = StateType::AUDIO_FOCUS_STATE_GAIN_TRANSIENT;
        break;

      case ReqType::AUDIO_FOCUS_RELEASE:
        // AA is done — HU media may resume.
        monitorState  = FocusState::Released;
        responseState = StateType::AUDIO_FOCUS_STATE_LOSS;
        break;

      default:
        monitorState  = FocusState::Gained;
        responseState = StateType::AUDIO_FOCUS_STATE_GAIN;
        break;
    }

    androidAutoMonitor_->onAudioFocusChanged(monitorState);
    qDebug(lcSession) << "audio focus response=" << AudioFocusStateType_Name(responseState);

    aap_protobuf::service::control::message::AudioFocusNotification response;
    response.set_focus_state(responseState);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  [capture0 = this->shared_from_this()](auto &&PH1) {
                    capture0->onChannelError(std::forward<decltype(PH1)>(PH1));
                  });
    controlServiceChannel_->sendAudioFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onByeByeRequest(
      const aap_protobuf::service::control::message::ByeByeRequest &request) {
    qInfo(lcSession) << "bye-bye request reason=" << request.reason();

    aap_protobuf::service::control::message::ByeByeResponse response;
    auto promise = aasdk::channel::SendPromise::defer();
    promise->then(std::bind(&AndroidAutoSession::triggerQuit, this->shared_from_this()),
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));

    controlServiceChannel_->sendShutdownResponse(response, std::move(promise));
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);
  }

  void AndroidAutoSession::onByeByeResponse(
      const aap_protobuf::service::control::message::ByeByeResponse &response) {
    qInfo(lcSession) << "bye-bye acknowledged, quitting";
    androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);
    this->triggerQuit();
  }

  void AndroidAutoSession::onNavigationFocusRequest(
      const aap_protobuf::service::control::message::NavFocusRequestNotification &request) {
    qDebug(lcSession) << "nav focus request type=" << NavFocusType_Name(request.focus_type());

    // TODO(nav-focus — post-5.1): Hardcoded to NAV_FOCUS_PROJECTED (AA owns navigation).
    // When JourneyOS has its own navigation source, this must become stateful:
    //   - If HU nav is active → respond NAV_FOCUS_NATIVE (HU keeps nav; AA nav suppressed)
    //   - If AA nav requested → respond NAV_FOCUS_PROJECTED (AA takes nav; suppress HU nav)
    // The HU should also proactively send NAV_FOCUS_NATIVE when starting its own navigation.
    aap_protobuf::service::control::message::NavFocusNotification response;
    response.set_focus_type(
        aap_protobuf::service::control::message::NavFocusType::NAV_FOCUS_PROJECTED);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendNavigationFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onBatteryStatusNotification(
      const aap_protobuf::service::control::message::BatteryStatusNotification &notification) {
    qDebug(lcSession) << "battery status notification received";
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onPingRequest(const aap_protobuf::service::control::message::PingRequest &request) {
    qDebug(lcSession) << "ping request ts=" << request.timestamp();

    // On the very first ping, forward the phone's timestamp to TimeController via
    // AndroidAutoMonitor. The signal is queued to the main Qt thread automatically.
    if (!m_phoneTimeOffered && request.timestamp() > 0) {
        androidAutoMonitor_->notifyPhoneTimestamp(static_cast<quint64>(request.timestamp()));
        m_phoneTimeOffered = true;
    }

    aap_protobuf::service::control::message::PingResponse response;
    response.set_timestamp(request.timestamp());

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    controlServiceChannel_->sendPingResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onVoiceSessionRequest(
      const aap_protobuf::service::control::message::VoiceSessionNotification &request) {
    qDebug(lcSession) << "voice session request received";
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onPingResponse(const aap_protobuf::service::control::message::PingResponse &response) {
    qDebug(lcSession) << "ping response ts=" << response.timestamp();
    pinger_->pong();
    controlServiceChannel_->receive(this->shared_from_this());
  }

  void AndroidAutoSession::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcSession) << "channel error:" << e.what();
    this->triggerQuit();
  }

  void AndroidAutoSession::triggerQuit() {
    qInfo(lcSession) << "<<< session quit triggered";
    if (eventHandler_ != nullptr) {
      eventHandler_->onAndroidAutoQuit();
    }
  }

  void AndroidAutoSession::schedulePing() {
    qDebug(lcSession) << "ping scheduled";
    auto promise = IPinger::Promise::defer();
    promise->then([this, self = this->shared_from_this()]() {
                    this->sendPing();
                    this->schedulePing();
                  },
                  [this, self = this->shared_from_this()](auto error) {
                    if (error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
                        error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                      qCritical(lcSession) << "ping timeout exceeded";
                      this->triggerQuit();
                    }
                  });

    pinger_->ping(std::move(promise));
  }

  void AndroidAutoSession::sendPing() {
    qDebug(lcSession) << "sending ping";
    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&AndroidAutoSession::onChannelError, this->shared_from_this(), std::placeholders::_1));

    aap_protobuf::service::control::message::PingRequest request;
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
    request.set_timestamp(timestamp.count());
    controlServiceChannel_->sendPingRequest(request, std::move(promise));
  }
}



