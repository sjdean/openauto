#pragma once
#include <aasdk/Channel/GenericNotification/GenericNotificationService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <boost/asio/io_service.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

namespace f1x::openauto::autoapp::service::genericnotification {
    class GenericNotificationService :
            public aasdk::channel::genericnotification::IGenericNotificationServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<GenericNotificationService> {
    public:
        GenericNotificationService(boost::asio::io_service &ioService, aasdk::messenger::IMessenger::Pointer messenger);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<GenericNotificationService>::shared_from_this;
        boost::asio::deadline_timer timer_;
        boost::asio::io_service::strand strand_;
        aasdk::channel::genericnotification::GenericNotificationService::Pointer channel_;
    };
}
