#pragma once
#include <QTimer>
#include <aasdk/Channel/NavigationStatus/NavigationStatusService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

namespace f1x::openauto::autoapp::service::navigationstatus {
    class NavigationStatusService :
            public aasdk::channel::navigationstatus::INavigationStatusServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<NavigationStatusService> {
    public:
        NavigationStatusService(aasdk::messenger::IMessenger::Pointer messenger);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

        void onStatusUpdate(
            const aap_protobuf::service::navigationstatus::message::NavigationStatus &navStatus) override;

        void onTurnEvent(
            const aap_protobuf::service::navigationstatus::message::NavigationNextTurnEvent &turnEvent) override;

        void onDistanceEvent(
            const aap_protobuf::service::navigationstatus::message::NavigationNextTurnDistanceEvent &
            distanceEvent) override;

    private:
        using std::enable_shared_from_this<NavigationStatusService>::shared_from_this;
        QTimer timer_;
        boost::asio::io_service::strand strand_;
        aasdk::channel::navigationstatus::NavigationStatusService::Pointer channel_;
    };
}
