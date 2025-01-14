#pragma once
#include <memory>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityEventHandler.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class IAndroidAutoEntity
{
public:
    typedef std::shared_ptr<IAndroidAutoEntity> Pointer;

    virtual ~IAndroidAutoEntity() = default;

    virtual void start(IAndroidAutoEntityEventHandler& eventHandler) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
};

}
}
}
}
