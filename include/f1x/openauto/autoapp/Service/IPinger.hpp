

#pragma once#include <aasdk/IO/Promise.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class IPinger
{
public:
    typedef std::shared_ptr<IPinger> Pointer;
    typedef aasdk::io::Promise<void> Promise;

    virtual ~IPinger() = default;
    virtual void ping(Promise::Pointer promise) = 0;
    virtual void pong() = 0;
    virtual void cancel() = 0;
};

}
}
}
}
