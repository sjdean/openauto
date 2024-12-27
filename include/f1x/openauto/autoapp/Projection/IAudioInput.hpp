

#pragma once#include <memory>
#include <aasdk/IO/Promise.hpp>
#include <aasdk/Common/Data.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class IAudioInput
{
public:
    typedef aasdk::io::Promise<void, void> StartPromise;
    typedef aasdk::io::Promise<aasdk::common::Data, void> ReadPromise;
    typedef std::shared_ptr<IAudioInput> Pointer;

    virtual ~IAudioInput() = default;

    virtual bool open() = 0;
    virtual bool isActive() const = 0;
    virtual void read(ReadPromise::Pointer promise) = 0;
    virtual void start(StartPromise::Pointer promise) = 0;
    virtual void stop() = 0;
    virtual uint32_t getSampleSize() const = 0;
    virtual uint32_t getChannelCount() const = 0;
    virtual uint32_t getSampleRate() const = 0;
};

}
}
}
}
