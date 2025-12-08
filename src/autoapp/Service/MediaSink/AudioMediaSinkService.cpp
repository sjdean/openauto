#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkMediaAudio, "journeyos.service.mediasink.audiomedia")

namespace f1x::openauto::autoapp::service::mediasink {

  AudioMediaSinkService::AudioMediaSinkService(boost::asio::io_service &ioService,
                                               aasdk::channel::mediasink::audio::IAudioMediaSinkService::Pointer channel,
                                               projection::IAudioOutput::Pointer audioOutput)
      : strand_(ioService), channel_(std::move(channel)), audioOutput_(std::move(audioOutput)), session_(-1) {

  }

  void AudioMediaSinkService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] start()";
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());
      channel_->receive(this->shared_from_this());
    });
  }

  void AudioMediaSinkService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] stop()";
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());
      audioOutput_->stop();
    });
  }

  void AudioMediaSinkService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] pause()";
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());

    });
  }

  void AudioMediaSinkService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] resume()";
      qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());

    });
  }

  /*
   * Service Discovery
   */

  void AudioMediaSinkService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] fillFeatures()";
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel: " << aasdk::messenger::channelIdToString(channel_->getId());

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto audioChannel = service->mutable_media_sink_service();

    audioChannel->set_available_type(
        aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

    switch (channel_->getId()) {
      case aasdk::messenger::ChannelId::MEDIA_SINK_SYSTEM_AUDIO:
        qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] System Audio.";
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_SYSTEM_AUDIO);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO:
        qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Music Audio.";
        audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_MEDIA);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_GUIDANCE_AUDIO:
        qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Guidance Audio.";
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_GUIDANCE);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_TELEPHONY_AUDIO:
        qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Telephony Audio.";
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_TELEPHONY);
        break;
      default:
        qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Unknown Audio.";
        break;
    }

    audioChannel->set_available_while_in_call(true);

    auto *audioConfig = audioChannel->add_audio_configs();
    audioConfig->set_sampling_rate(audioOutput_->getSampleRate());
    audioConfig->set_number_of_bits(audioOutput_->getSampleSize());
    audioConfig->set_number_of_channels(audioOutput_->getChannelCount());

    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] getSampleRate " << audioOutput_->getSampleRate();
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] getSampleSize " << audioOutput_->getSampleSize();
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] getChannelCount " << audioOutput_->getChannelCount();
    //qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] SampleRate " << audioConfig->sampling_rate() << " / " << audioConfig->number_of_bits() << " / " << audioConfig->number_of_channels();
  }

  /*
   * Base Channel Handling
   */

  void AudioMediaSinkService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onChannelOpenRequest()";
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Sample Rate: " << audioOutput_->getSampleRate() << ", Sample Size: "
                       << audioOutput_->getSampleSize() << ", Audio Channels: " << audioOutput_->getChannelCount();

    const aap_protobuf::shared::MessageStatus status = audioOutput_->open()
                                                       ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                       : aap_protobuf::shared::MessageStatus::STATUS_INVALID_CHANNEL;

    qDebug(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Status determined: "
                        << aap_protobuf::shared::MessageStatus_Name(status);

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onChannelError(): " << e.what()
                        << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());
  }

  /*
   * Media Channel Handling
   */

  void AudioMediaSinkService::onMediaChannelSetupRequest(
      const aap_protobuf::service::media::shared::message::Setup &request) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onMediaChannelSetupRequest()";
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", Codec: "
                       << MediaCodecType_Name(request.type());

    aap_protobuf::service::media::shared::message::Config response;
    auto status = aap_protobuf::service::media::shared::message::Config::STATUS_READY;
    response.set_status(status);
    response.set_max_unacked(1);
    response.add_configuration_indices(0);

    auto promise = aasdk::channel::SendPromise::defer(strand_);

    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelSetupResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }


  void AudioMediaSinkService::onMediaChannelStartIndication(
      const aap_protobuf::service::media::shared::message::Start &indication) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onMediaChannelStartIndication()";
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: "
                       << indication.session_id();
    session_ = indication.session_id();
    audioOutput_->start();
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaChannelStopIndication(
      const aap_protobuf::service::media::shared::message::Stop &indication) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onMediaChannelStopIndication()";
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

    session_ = -1;
    audioOutput_->suspend();

    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                             const aasdk::common::DataConstBuffer &buffer) {
    qDebug(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onMediaWithTimestampIndication()";
    qDebug(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] Channel Id: "
                        << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

    audioOutput_->write(timestamp, buffer);

    aap_protobuf::service::media::source::message::Ack indication;
    indication.set_session_id(session_);
    indication.set_ack(1);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendMediaAckIndication(indication, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
    qInfo(lcServiceSinkMediaAudio) << "[AudioMediaSinkService] onMediaIndication()";

    this->onMediaWithTimestampIndication(0, buffer);
  }
}
