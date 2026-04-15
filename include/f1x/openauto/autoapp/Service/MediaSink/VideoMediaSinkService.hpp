#pragma once
#include <aasdk/Messenger/IMessenger.hpp>
#include <aasdk/Channel/MediaSink/Video/IVideoMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Video/IVideoMediaSinkServiceEventHandler.hpp>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x::openauto::autoapp::service::mediasink {
    class VideoMediaSinkService :
            public aasdk::channel::mediasink::video::IVideoMediaSinkServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<VideoMediaSinkService> {
    public:
        typedef std::shared_ptr<VideoMediaSinkService> Pointer;

        // General Constructor
        VideoMediaSinkService(aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel,
                              projection::IVideoOutput::Pointer videoOutput);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onMediaChannelSetupRequest(
            const aap_protobuf::service::media::shared::message::Setup &request) override;

        void onMediaChannelStartIndication(
            const aap_protobuf::service::media::shared::message::Start &indication) override;

        void onMediaChannelStopIndication(
            const aap_protobuf::service::media::shared::message::Stop &indication) override;

        void onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                            const aasdk::common::DataConstBuffer &buffer) override;

        void onMediaIndication(const aasdk::common::DataConstBuffer &buffer) override;

        void onChannelError(const aasdk::error::Error &e) override;

        void onVideoFocusRequest(
            const aap_protobuf::service::media::video::message::VideoFocusRequestNotification &request) override;

        void sendVideoFocusIndication();

    protected:
        using std::enable_shared_from_this<VideoMediaSinkService>::shared_from_this;
        aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel_;
        projection::IVideoOutput::Pointer videoOutput_;
        int32_t session_;
    };
}
