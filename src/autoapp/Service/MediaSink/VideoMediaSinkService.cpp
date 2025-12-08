#include <fstream>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoMediaSinkService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkMediaVideo, "journeyos.service.mediasink.videomedia")

namespace f1x::openauto::autoapp::service::mediasink {
  VideoMediaSinkService::VideoMediaSinkService(boost::asio::io_service &ioService,
                                               aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel,
                                               projection::IVideoOutput::Pointer videoOutput)
      : strand_(ioService), channel_(std::move(channel)), videoOutput_(std::move(videoOutput)), session_(-1) {

  }

  void VideoMediaSinkService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] start()";
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());
      channel_->receive(this->shared_from_this());
    });
  }

  void VideoMediaSinkService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] stop()";
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());
      videoOutput_->stop();
    });
  }

  void VideoMediaSinkService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] pause()";
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());
    });
  }

  void VideoMediaSinkService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] resume()";
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel "
                         << aasdk::messenger::channelIdToString(channel_->getId());

    });
  }

  void VideoMediaSinkService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] fillFeatures()";
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel "
                       << aasdk::messenger::channelIdToString(channel_->getId());

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *videoChannel = service->mutable_media_sink_service();

    videoChannel->set_available_type(
        aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_VIDEO_H264_BP);
    videoChannel->set_available_while_in_call(true);


    auto *videoConfig1 = videoChannel->add_video_configs();
    videoConfig1->set_codec_resolution(videoOutput_->getVideoResolution());
    videoConfig1->set_frame_rate(videoOutput_->getVideoFPS());

    const auto &videoMargins = videoOutput_->getVideoMargins();
    videoConfig1->set_height_margin(videoMargins.height());
    videoConfig1->set_width_margin(videoMargins.width());
    videoConfig1->set_density(videoOutput_->getScreenDPI());

    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] getVideoResolution "
                       << VideoCodecResolutionType_Name(videoOutput_->getVideoResolution());
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] getVideoFPS "
                       << VideoFrameRateType_Name(videoOutput_->getVideoFPS());
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] width " << videoMargins.width();
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] height " << videoMargins.height();
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] getScreenDPI " << videoOutput_->getScreenDPI();
  }

  void
  VideoMediaSinkService::onMediaChannelSetupRequest(
      const aap_protobuf::service::media::shared::message::Setup &request) {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onMediaChannelSetupRequest()";
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", Codec: "
                       << MediaCodecType_Name(request.type());


    auto status = videoOutput_->init()
                  ? aap_protobuf::service::media::shared::message::Config::STATUS_READY
                  : aap_protobuf::service::media::shared::message::Config::STATUS_WAIT;

    qDebug(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] setup status: " << Config_Status_Name(status);

    aap_protobuf::service::media::shared::message::Config response;
    response.set_status(status);
    response.set_max_unacked(1);
    response.add_configuration_indices(0);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then(std::bind(&VideoMediaSinkService::sendVideoFocusIndication, this->shared_from_this()),
                  std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                            std::placeholders::_1));

    channel_->sendChannelSetupResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onChannelOpenRequest()";
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    const aap_protobuf::shared::MessageStatus status = videoOutput_->open()
                                                       ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                       : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Status determined: "
                       << aap_protobuf::shared::MessageStatus_Name(status);

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::onMediaChannelStartIndication(
      const aap_protobuf::service::media::shared::message::Start &indication) {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onMediaChannelStartIndication()";
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: "
                       << indication.session_id();

    session_ = indication.session_id();
    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::onMediaChannelStopIndication(
      const aap_protobuf::service::media::shared::message::Stop &indication) {
    qInfo(lcServiceSinkMediaVideo) << "[onMediaChannelStopIndication] onMediaChannelStopIndication()";
    qInfo(lcServiceSinkMediaVideo) << "[onMediaChannelStopIndication] Channel Id: "
                       << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                             const aasdk::common::DataConstBuffer &buffer) {
    qDebug(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onMediaWithTimestampIndication()";
    qDebug(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Channel Id: "
                        << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

    videoOutput_->write(timestamp, buffer);

    aap_protobuf::service::media::source::message::Ack indication;
    indication.set_session_id(session_);
    indication.set_ack(1);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendMediaAckIndication(indication, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
    qDebug(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onMediaIndication()";
    this->onMediaWithTimestampIndication(0, buffer);
  }

  void VideoMediaSinkService::onChannelError(const aasdk::error::Error &e) {
    qCritical(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onChannelError(): " << e.what()
                        << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());
  }

  void VideoMediaSinkService::onVideoFocusRequest(
      const aap_protobuf::service::media::video::message::VideoFocusRequestNotification &request) {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] onVideoFocusRequest()";
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Display index: " << request.disp_channel_id() << ", focus mode: "
                       << VideoFocusMode_Name(request.mode()) << ", focus reason: "
                       << VideoFocusReason_Name(request.reason());

    if (request.mode() ==
        aap_protobuf::service::media::video::message::VideoFocusMode::VIDEO_FOCUS_NATIVE) {
      // Return to OS
      qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Returning to OS.";
      try {
        if (!std::ifstream("/tmp/entityexit")) {
          std::ofstream("/tmp/entityexit");
        }
      } catch (...) {
        qCritical(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] Error in creating /tmp/entityexit";
      }
    }

    this->sendVideoFocusIndication();
    channel_->receive(this->shared_from_this());
  }

  void VideoMediaSinkService::sendVideoFocusIndication() {
    qInfo(lcServiceSinkMediaVideo) << "[VideoMediaSinkService] sendVideoFocusIndication()";

    aap_protobuf::service::media::video::message::VideoFocusNotification videoFocusIndication;
    videoFocusIndication.set_focus(
        aap_protobuf::service::media::video::message::VideoFocusMode::VIDEO_FOCUS_PROJECTED);
    videoFocusIndication.set_unsolicited(false);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendVideoFocusIndication(videoFocusIndication, std::move(promise));
  }
}
