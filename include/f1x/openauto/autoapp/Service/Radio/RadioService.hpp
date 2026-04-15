#pragma once
#include <QTimer>
#include <aasdk/Channel/Radio/RadioService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

namespace f1x::openauto::autoapp::service::radio {
    class RadioService :
            public aasdk::channel::radio::IRadioServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<RadioService> {
    public:
        RadioService(aasdk::messenger::IMessenger::Pointer messenger);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<RadioService>::shared_from_this;
        QTimer timer_;
        boost::asio::io_service::strand strand_;
        aasdk::channel::radio::RadioService::Pointer channel_;
    };
}
