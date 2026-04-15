#pragma once
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <aasdk/Channel/InputSource/InputSourceService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <f1x/openauto/autoapp/Projection/IInputDevice.hpp>
#include <f1x/openauto/autoapp/Projection/IInputDeviceEventHandler.hpp>

namespace f1x::openauto::autoapp::service::inputsource {
    class InputSourceService :
            public aasdk::channel::inputsource::IInputSourceServiceEventHandler,
            public IService,
            public projection::IInputDeviceEventHandler,
            public std::enable_shared_from_this<InputSourceService> {
    public:
        InputSourceService(aasdk::messenger::IMessenger::Pointer messenger,
                           projection::IInputDevice::Pointer inputDevice);

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;


        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onKeyBindingRequest(
            const aap_protobuf::service::media::sink::message::KeyBindingRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

        void onButtonEvent(const projection::ButtonEvent &event) override;

        void onTouchEvent(const projection::TouchEvent &event) override;

    private:
        using std::enable_shared_from_this<InputSourceService>::shared_from_this;

        boost::asio::io_service::strand strand_;
        aasdk::channel::inputsource::InputSourceService::Pointer channel_;
        projection::IInputDevice::Pointer inputDevice_;
    };
}
