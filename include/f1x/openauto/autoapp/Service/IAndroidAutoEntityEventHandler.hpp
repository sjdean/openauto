#pragma once
#include <aasdk/Error/Error.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class IAndroidAutoEntityEventHandler
{
public:
    virtual ~IAndroidAutoEntityEventHandler() = default;
    virtual void onAndroidAutoQuit() = 0;
};

}
}
}
}
