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
      qInfo(lcServiceSinkMediaAudio) << "starting channel=" << aasdk::messenger::channelIdToString(channel_->getId());
      channel_->receive(this->shared_from_this());
    });
  }

  void AudioMediaSinkService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "stopping channel=" << aasdk::messenger::channelIdToString(channel_->getId());
      audioOutput_->stop();
    });
  }

  void AudioMediaSinkService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "paused channel=" << aasdk::messenger::channelIdToString(channel_->getId());
    });
  }

  void AudioMediaSinkService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaAudio) << "resumed channel=" << aasdk::messenger::channelIdToString(channel_->getId());
    });
  }

  /*
   * Service Discovery
   */

  void AudioMediaSinkService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto audioChannel = service->mutable_media_sink_service();

    audioChannel->set_available_type(
        aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

    switch (channel_->getId()) {
      case aasdk::messenger::ChannelId::MEDIA_SINK_SYSTEM_AUDIO:
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_SYSTEM_AUDIO);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO:
        audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_MEDIA);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_GUIDANCE_AUDIO:
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_GUIDANCE);
        break;

      case aasdk::messenger::ChannelId::MEDIA_SINK_TELEPHONY_AUDIO:
        audioChannel->set_audio_type(
            aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_TELEPHONY);
        break;
      default:
        qWarning(lcServiceSinkMediaAudio) << "unknown audio channel=" << aasdk::messenger::channelIdToString(channel_->getId());
        break;
    }

    audioChannel->set_available_while_in_call(true);

    // Advertise static supported configs. Sink creation is deferred until Setup
    // (onMediaChannelSetupRequest), so these are not read from the output object.
    const bool isMedia = (channel_->getId() == aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO);
    const uint32_t channels = isMedia ? 2 : 1;

    // Config 0: 48000 Hz (primary — we always confirm this index in Setup response).
    auto *cfg0 = audioChannel->add_audio_configs();
    cfg0->set_sampling_rate(48000);
    cfg0->set_number_of_bits(16);
    cfg0->set_number_of_channels(channels);

    // Config 1: lower-rate alternative (44100 for media, 16000 for voice channels).
    auto *cfg1 = audioChannel->add_audio_configs();
    cfg1->set_sampling_rate(isMedia ? 44100 : 16000);
    cfg1->set_number_of_bits(16);
    cfg1->set_number_of_channels(channels);

    qInfo(lcServiceSinkMediaAudio) << "channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                   << " advertising 48000+" << (isMedia ? 44100 : 16000)
                                   << " Hz" << channels << "ch 16-bit";
  }

  /*
   * Base Channel Handling
   */

  void AudioMediaSinkService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    // Format negotiation happens in onMediaChannelSetupRequest; just acknowledge here.
    qInfo(lcServiceSinkMediaAudio) << "channel open service_id=" << request.service_id();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceSinkMediaAudio) << "channel error=" << e.what()
                                       << " channel=" << aasdk::messenger::channelIdToString(channel_->getId());
  }

  /*
   * Media Channel Handling
   */

  void AudioMediaSinkService::onMediaChannelSetupRequest(
      const aap_protobuf::service::media::shared::message::Setup &request) {
    qInfo(lcServiceSinkMediaAudio) << "setup channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                   << " codec=" << MediaCodecType_Name(request.type());

    // Configure and open the sink now that the format is confirmed (index 0 = 48000 Hz).
    const bool isMedia = (channel_->getId() == aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO);
    audioOutput_->setFormat(isMedia ? 2 : 1, 16, 48000);
    audioOutput_->open();

    aap_protobuf::service::media::shared::message::Config response;
    response.set_status(aap_protobuf::service::media::shared::message::Config::STATUS_READY);
    response.set_max_unacked(1);
    response.add_configuration_indices(0); // confirm index 0 (48000 Hz)

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelSetupResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaChannelStartIndication(
      const aap_protobuf::service::media::shared::message::Start &indication) {
    qInfo(lcServiceSinkMediaAudio) << "media start channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                   << " session=" << indication.session_id();
    session_ = indication.session_id();
    audioOutput_->start();
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaChannelStopIndication(
      const aap_protobuf::service::media::shared::message::Stop &indication) {
    qInfo(lcServiceSinkMediaAudio) << "media stop channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                   << " session=" << session_;
    session_ = -1;
    audioOutput_->suspend();
    channel_->receive(this->shared_from_this());
  }

  void AudioMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                             const aasdk::common::DataConstBuffer &buffer) {
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
    this->onMediaWithTimestampIndication(0, buffer);
  }
}
