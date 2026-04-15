#pragma once
#include <QTimer>
#include <aasdk/Channel/MediaBrowser/MediaBrowserService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

namespace f1x::openauto::autoapp::service::mediabrowser {
    class MediaBrowserService :
            public aasdk::channel::mediabrowser::IMediaBrowserServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<MediaBrowserService> {
    public:
        MediaBrowserService(aasdk::messenger::IMessenger::Pointer messenger);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<MediaBrowserService>::shared_from_this;
        QTimer timer_;
        boost::asio::io_service::strand strand_;
        aasdk::channel::mediabrowser::MediaBrowserService::Pointer channel_;
    };
}
