

#pragma once#include <memory>
#include <aasdk/Messenger/Timestamp.hpp>
#include <aasdk/Common/Data.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class IAudioOutput
{
public:
    typedef std::shared_ptr<IAudioOutput> Pointer;

    IAudioOutput() = default;
    virtual ~IAudioOutput() = default;

    virtual bool open() = 0;
    virtual void write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer& buffer) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void suspend() = 0;
    virtual uint32_t getSampleSize() const = 0;
    virtual uint32_t getChannelCount() const = 0;
    virtual uint32_t getSampleRate() const = 0;
};

}
}
}
}
