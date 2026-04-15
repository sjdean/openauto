#pragma once
#include <QTimer>
#include <aasdk/Channel/WifiProjection/WifiProjectionService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <aasdk/Messenger/IMessenger.hpp>


namespace f1x::openauto::autoapp::service::wifiprojection {
    class WifiProjectionService :
            public aasdk::channel::wifiprojection::IWifiProjectionServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<WifiProjectionService> {
    public:
        WifiProjectionService(aasdk::messenger::IMessenger::Pointer messenger,
                              configuration::IConfiguration::Pointer configuration);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

        void
        onWifiCredentialsRequest(
            const aap_protobuf::service::wifiprojection::message::WifiCredentialsRequest &request) override;

    private:
        using std::enable_shared_from_this<WifiProjectionService>::shared_from_this;
        configuration::IConfiguration::Pointer configuration_;
        QTimer timer_;
        boost::asio::io_service::strand strand_;
        aasdk::channel::wifiprojection::WifiProjectionService::Pointer channel_;
    };
}
