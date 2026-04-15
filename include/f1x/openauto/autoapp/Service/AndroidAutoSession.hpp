#pragma once
#include <boost/asio.hpp>
#include <aasdk/Transport/ITransport.hpp>
#include <aasdk/Channel/Control/IControlServiceChannel.hpp>
#include <aasdk/Channel/Control/IControlServiceChannelEventHandler.hpp>
#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoSession.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <f1x/openauto/autoapp/Service/IPinger.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::service {
    class AndroidAutoSession : public IAndroidAutoSession,
                              public aasdk::channel::control::IControlServiceChannelEventHandler,
                              public std::enable_shared_from_this<AndroidAutoSession> {
    public:
        AndroidAutoSession(aasdk::messenger::ICryptor::Pointer cryptor,
                          aasdk::transport::ITransport::Pointer transport,
                          aasdk::messenger::IMessenger::Pointer messenger,
                          configuration::IConfiguration::Pointer configuration,
                          ServiceList serviceList,
                          IPinger::Pointer pinger,
                          std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor);

        ~AndroidAutoSession() override;

        void start(IAndroidAutoSessionEventHandler &eventHandler) override;

        void stop() override;

        void pause() override;

        void resume() override;

        void onVersionResponse(uint16_t majorCode, uint16_t minorCode,
                               aap_protobuf::shared::MessageStatus status) override;

        void onHandshake(const aasdk::common::DataConstBuffer &payload) override;

        void onServiceDiscoveryRequest(
            const aap_protobuf::service::control::message::ServiceDiscoveryRequest &request) override;

        void onAudioFocusRequest(const aap_protobuf::service::control::message::AudioFocusRequest &request) override;

        void onByeByeRequest(const aap_protobuf::service::control::message::ByeByeRequest &request) override;

        void onByeByeResponse(const aap_protobuf::service::control::message::ByeByeResponse &response) override;

        void onNavigationFocusRequest(
            const aap_protobuf::service::control::message::NavFocusRequestNotification &request) override;

        void onVoiceSessionRequest(
            const aap_protobuf::service::control::message::VoiceSessionNotification &request) override;

        void onBatteryStatusNotification(
            const aap_protobuf::service::control::message::BatteryStatusNotification &notification) override;

        void onPingResponse(const aap_protobuf::service::control::message::PingResponse &response) override;

        void onPingRequest(const aap_protobuf::service::control::message::PingRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<AndroidAutoSession>::shared_from_this;

        void triggerQuit();

        void schedulePing();

        void sendPing();

        boost::asio::io_service::strand strand_;
        aasdk::messenger::ICryptor::Pointer cryptor_;
        aasdk::transport::ITransport::Pointer transport_;
        aasdk::messenger::IMessenger::Pointer messenger_;
        aasdk::channel::control::IControlServiceChannel::Pointer controlServiceChannel_;
        configuration::IConfiguration::Pointer configuration_;
        ServiceList serviceList_;
        IPinger::Pointer pinger_;
        IAndroidAutoSessionEventHandler *eventHandler_;
        std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor_;
        // True after the first PingRequest timestamp has been forwarded to TimeController.
        // Prevents offering the phone time on every subsequent ping interval.
        bool m_phoneTimeOffered{false};
    };
}
