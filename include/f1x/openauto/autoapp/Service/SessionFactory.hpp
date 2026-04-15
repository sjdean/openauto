#pragma once

#include <boost/asio.hpp>
#include <aasdk/Transport/ITransport.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/ISessionFactory.hpp>
#include <f1x/openauto/autoapp/Service/IServiceFactory.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::service {
    class SessionFactory : public ISessionFactory {
    public:
        SessionFactory(configuration::IConfiguration::Pointer configuration,
                                 IServiceFactory &serviceFactory,
                                 std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor);

        IAndroidAutoSession::Pointer create(aasdk::usb::IAOAPDevice::Pointer aoapDevice) override;

        IAndroidAutoSession::Pointer create(aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint) override;

    private:
        IAndroidAutoSession::Pointer create(aasdk::transport::ITransport::Pointer transport);

        ;
        configuration::IConfiguration::Pointer configuration_;
        IServiceFactory &serviceFactory_;
        std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor_;
    };
}
