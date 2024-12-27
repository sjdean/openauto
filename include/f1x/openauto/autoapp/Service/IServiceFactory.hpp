

#pragma once#include <aasdk/Messenger/IMessenger.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class IServiceFactory
{
public:
    virtual ~IServiceFactory() = default;

    virtual ServiceList create(aasdk::messenger::IMessenger::Pointer messenger) = 0;
};

}
}
}
}
