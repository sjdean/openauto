#pragma once
#include <aasdk/TCP/ITCPEndpoint.hpp>
#include <aasdk/USB/IAOAPDevice.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoSession.hpp>

namespace f1x::openauto::autoapp::service {
    class ISessionFactory {
    public:
        virtual ~ISessionFactory() = default;

        virtual IAndroidAutoSession::Pointer create(aasdk::usb::IAOAPDevice::Pointer aoapDevice) = 0;

        virtual IAndroidAutoSession::Pointer create(aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint) = 0;
    };
}
