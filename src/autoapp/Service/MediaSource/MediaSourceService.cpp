#include <f1x/openauto/autoapp/Service/MediaSource/MediaSourceService.hpp>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSourceMedia, "journeyos.service.mediasource.media")

namespace f1x::openauto::autoapp::service::mediasource {

  MediaSourceService::MediaSourceService(aasdk::channel::mediasource::IMediaSourceService::Pointer channel,
                                         projection::IAudioInput::Pointer audioInput)
      : channel_(std::move(channel)), audioInput_(std::move(audioInput)), session_(-1) {

  }

  void MediaSourceService::start() {
      qInfo(lcServiceSourceMedia) << "starting";
  channel_->receive(this->shared_from_this());
  }

  void MediaSourceService::stop() {
      qInfo(lcServiceSourceMedia) << "stopping";
  audioInput_->stop();
  }

  void MediaSourceService::pause() {
      qInfo(lcServiceSourceMedia) << "paused";
  }

  void MediaSourceService::resume() {
      qInfo(lcServiceSourceMedia) << "resumed";
  }

  /*
   * Service Discovery
   */

  void MediaSourceService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *avInputChannel = service->mutable_media_source_service();
    avInputChannel->set_available_type(
        aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

    auto audioConfig = avInputChannel->mutable_audio_config();
    audioConfig->set_sampling_rate(audioInput_->getSampleRate());
    audioConfig->set_number_of_bits(audioInput_->getSampleSize());
    audioConfig->set_number_of_channels(audioInput_->getChannelCount());
  }

  /*
   * Base Channel Handling
   */

  void MediaSourceService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    const aap_protobuf::shared::MessageStatus status = audioInput_->open()
                                                       ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                       : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

    qInfo(lcServiceSourceMedia) << "channel open service_id=" << request.service_id()
                                << " status=" << aap_protobuf::shared::MessageStatus_Name(status);

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                            std::placeholders::_1));

    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void MediaSourceService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceSourceMedia) << "channel error=" << e.what();
  }

  /*
   * Media Channel Handling
   */

  void MediaSourceService::onMediaChannelSetupRequest(
      const aap_protobuf::service::media::shared::message::Setup &request) {
    qInfo(lcServiceSourceMedia) << "setup channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                << " codec=" << MediaCodecType_Name(request.type());

    aap_protobuf::service::media::shared::message::Config response;
    response.set_status(aap_protobuf::service::media::shared::message::Config::STATUS_READY);
    response.set_max_unacked(1);
    response.add_configuration_indices(0);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelSetupResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void MediaSourceService::onMediaChannelAckIndication(
      const aap_protobuf::service::media::source::message::Ack &) {
    channel_->receive(this->shared_from_this());
  }

  /*
   * Source Media Channel Handling
   */

  void MediaSourceService::onMediaSourceOpenRequest(
      const aap_protobuf::service::media::source::message::MicrophoneRequest &request) {
    qInfo(lcServiceSourceMedia) << "mic request open=" << request.open()
                                << " anc=" << request.anc_enabled()
                                << " ec=" << request.ec_enabled()
                                << " max_unacked=" << request.max_unacked();

    if (request.open()) {
      auto startPromise = projection::IAudioInput::StartPromise::defer();
      startPromise->then(std::bind(&MediaSourceService::onMediaSourceOpenSuccess, this->shared_from_this()),
                         [this, self = this->shared_from_this()]() {
                           qCritical(lcServiceSourceMedia) << "mic open failed";

                           aap_protobuf::service::media::source::message::MicrophoneResponse response;
                           response.set_session_id(session_);
                           response.set_status(aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR);

                           auto sendPromise = aasdk::channel::SendPromise::defer();
                           sendPromise->then([]() {},
                                             std::bind(&MediaSourceService::onChannelError,
                                                       this->shared_from_this(),
                                                       std::placeholders::_1));
                           channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));
                         });

      audioInput_->start(std::move(startPromise));
    } else {
      audioInput_->stop();

      aap_protobuf::service::media::source::message::MicrophoneResponse response;
      response.set_session_id(session_);
      response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

      auto sendPromise = aasdk::channel::SendPromise::defer();
      sendPromise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                           std::placeholders::_1));
      channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));
    }

    channel_->receive(this->shared_from_this());
  }

  void MediaSourceService::onMediaSourceOpenSuccess() {
    qInfo(lcServiceSourceMedia) << "mic opened successfully";

    aap_protobuf::service::media::source::message::MicrophoneResponse response;
    response.set_session_id(session_);
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

    auto sendPromise = aasdk::channel::SendPromise::defer();
    sendPromise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                         std::placeholders::_1));

    channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));

    this->readMediaSource();
  }

  void MediaSourceService::onMediaSourceDataReady(aasdk::common::Data data) {
    auto sendPromise = aasdk::channel::SendPromise::defer();
    sendPromise->then(std::bind(&MediaSourceService::readMediaSource, this->shared_from_this()),
                      std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                std::placeholders::_1));

    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
    channel_->sendMediaSourceWithTimestampIndication(timestamp.count(), std::move(data), std::move(sendPromise));
  }

  void MediaSourceService::readMediaSource() {
    if (audioInput_->isActive()) {
      auto readPromise = projection::IAudioInput::ReadPromise::defer();
      readPromise->then(
          std::bind(&MediaSourceService::onMediaSourceDataReady, this->shared_from_this(),
                    std::placeholders::_1),
          [this, self = this->shared_from_this()]() {
            qDebug(lcServiceSourceMedia) << "audio read rejected";
          });

      audioInput_->read(std::move(readPromise));
    }
  }
}
