

#pragma once#include <aasdk/TCP/ITCPEndpoint.hpp>
#include <aasdk/USB/IAOAPDevice.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class IAndroidAutoEntityFactory
{
public:
    virtual ~IAndroidAutoEntityFactory() = default;

    virtual IAndroidAutoEntity::Pointer create(aasdk::usb::IAOAPDevice::Pointer aoapDevice) = 0;
    virtual IAndroidAutoEntity::Pointer create(aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint) = 0;
};

}
}
}
}
