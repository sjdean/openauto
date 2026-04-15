#pragma once
#include <QTimer>
#include <aasdk/Channel/MediaPlaybackStatus/MediaPlaybackStatusService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

namespace f1x::openauto::autoapp::service::mediaplaybackstatus {
    class MediaPlaybackStatusService :
            public aasdk::channel::mediaplaybackstatus::IMediaPlaybackStatusServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<MediaPlaybackStatusService> {
    public:
        MediaPlaybackStatusService(aasdk::messenger::IMessenger::Pointer messenger);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<MediaPlaybackStatusService>::shared_from_this;
        QTimer timer_;
        aasdk::channel::mediaplaybackstatus::MediaPlaybackStatusService::Pointer channel_;
    };
}
