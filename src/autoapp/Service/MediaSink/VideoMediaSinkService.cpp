#include <fstream>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoMediaSinkService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkMediaVideo, "journeyos.service.mediasink.videomedia")

namespace f1x::openauto::autoapp::service::mediasink {
    VideoMediaSinkService::VideoMediaSinkService(boost::asio::io_service &ioService,
                                                 aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer
                                                 channel,
                                                 projection::IVideoOutput::Pointer videoOutput)
        : strand_(ioService), channel_(std::move(channel)), videoOutput_(std::move(videoOutput)), session_(-1) {
    }

    void VideoMediaSinkService::start() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcServiceSinkMediaVideo) << "starting channel=" << aasdk::messenger::channelIdToString(channel_->getId());
            channel_->receive(this->shared_from_this());
        });
    }

    void VideoMediaSinkService::stop() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcServiceSinkMediaVideo) << "stopping channel=" << aasdk::messenger::channelIdToString(channel_->getId());
            videoOutput_->stop();
        });
    }

    void VideoMediaSinkService::pause() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcServiceSinkMediaVideo) << "paused channel=" << aasdk::messenger::channelIdToString(channel_->getId());
        });
    }

    void VideoMediaSinkService::resume() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcServiceSinkMediaVideo) << "resumed channel=" << aasdk::messenger::channelIdToString(channel_->getId());
        });
    }

    void VideoMediaSinkService::fillFeatures(
        aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
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
        videoConfig1->set_density(160);
        videoConfig1->set_video_codec_type(aap_protobuf::service::media::shared::message::MEDIA_CODEC_VIDEO_H264_BP);

        qInfo(lcServiceSinkMediaVideo) << "channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                << " resolution=" << VideoCodecResolutionType_Name(videoOutput_->getVideoResolution())
                << " fps=" << VideoFrameRateType_Name(videoOutput_->getVideoFPS())
                << " margin_w=" << videoMargins.width() << " margin_h=" << videoMargins.height()
                << " dpi=" << videoOutput_->getScreenDPI();
    }

    void VideoMediaSinkService::onMediaChannelSetupRequest(
        const aap_protobuf::service::media::shared::message::Setup &request) {
        auto status = videoOutput_->init()
                          ? aap_protobuf::service::media::shared::message::Config::STATUS_READY
                          : aap_protobuf::service::media::shared::message::Config::STATUS_WAIT;

        qInfo(lcServiceSinkMediaVideo) << "setup codec=" << MediaCodecType_Name(request.type())
                                       << " init_status=" << Config_Status_Name(status);

        aap_protobuf::service::media::shared::message::Config response;
        response.set_status(status);
        response.set_max_unacked(4);
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
        const aap_protobuf::shared::MessageStatus status = videoOutput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

        qInfo(lcServiceSinkMediaVideo) << "channel open priority=" << request.priority()
                << " channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                << " status=" << aap_protobuf::shared::MessageStatus_Name(status);

        aap_protobuf::service::control::message::ChannelOpenResponse response;
        response.set_status(status);

        auto promise = aasdk::channel::SendPromise::defer(strand_);
        promise->then([]() {
        }, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                     std::placeholders::_1));
        channel_->sendChannelOpenResponse(response, std::move(promise));
        channel_->receive(this->shared_from_this());
    }

    void VideoMediaSinkService::onMediaChannelStartIndication(
        const aap_protobuf::service::media::shared::message::Start &indication) {
        qInfo(lcServiceSinkMediaVideo) << "video start channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                       << " session=" << indication.session_id();
        session_ = indication.session_id();
        channel_->receive(this->shared_from_this());
    }

    void VideoMediaSinkService::onMediaChannelStopIndication(
        const aap_protobuf::service::media::shared::message::Stop &indication) {
        qInfo(lcServiceSinkMediaVideo) << "video stop channel=" << aasdk::messenger::channelIdToString(channel_->getId())
                                       << " session=" << session_;
        channel_->receive(this->shared_from_this());
    }

    void VideoMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                               const aasdk::common::DataConstBuffer &buffer) {
        static int frameCount = 0;
        ++frameCount;
        if (frameCount <= 5 || frameCount % 100 == 0)
            qDebug(lcServiceSinkMediaVideo) << "frame=" << frameCount << " bytes=" << buffer.size;

        videoOutput_->write(timestamp, buffer);

        aap_protobuf::service::media::source::message::Ack indication;
        indication.set_session_id(session_);
        indication.set_ack(1);

        auto promise = aasdk::channel::SendPromise::defer(strand_);
        promise->then([]() {
        }, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                     std::placeholders::_1));
        channel_->sendMediaAckIndication(indication, std::move(promise));
        channel_->receive(this->shared_from_this());
    }

    void VideoMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
        this->onMediaWithTimestampIndication(0, buffer);
    }

    void VideoMediaSinkService::onChannelError(const aasdk::error::Error &e) {
        qCritical(lcServiceSinkMediaVideo) << "channel error=" << e.what()
                << " channel=" << aasdk::messenger::channelIdToString(channel_->getId());
    }

    void VideoMediaSinkService::onVideoFocusRequest(
        const aap_protobuf::service::media::video::message::VideoFocusRequestNotification &request) {
        qInfo(lcServiceSinkMediaVideo) << "video focus request disp=" << request.disp_channel_id()
                << " mode=" << VideoFocusMode_Name(request.mode())
                << " reason=" << VideoFocusReason_Name(request.reason());

        if (request.mode() ==
            aap_protobuf::service::media::video::message::VideoFocusMode::VIDEO_FOCUS_NATIVE) {
            // Return to OS
            qInfo(lcServiceSinkMediaVideo) << "returning to OS";
            try {
                if (!std::ifstream("/tmp/entityexit")) {
                    std::ofstream("/tmp/entityexit");
                }
            } catch (...) {
                qCritical(lcServiceSinkMediaVideo) << "failed to create /tmp/entityexit";
            }
        }

        this->sendVideoFocusIndication();
        channel_->receive(this->shared_from_this());
    }

    void VideoMediaSinkService::sendVideoFocusIndication() {
        qInfo(lcServiceSinkMediaVideo) << "sending video focus indication";

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
