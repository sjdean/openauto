#ifdef USE_OMX
#pragma once

extern "C"
{
#include <ilclient.h>
}#include <mutex>
#include <condition_variable>
#include <thread>
#include <boost/circular_buffer.hpp>
#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class OMXVideoOutput: public VideoOutput
{
public:
    OMXVideoOutput(configuration::IConfiguration::Pointer configuration);

    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;

private:
    bool createComponents();
    bool initClock();
    bool setupTunnels();
    bool enablePortBuffers();
    bool setupDisplayRegion();

    std::mutex mutex_;
    bool isActive_;
    bool portSettingsChanged_;
    ILCLIENT_T* client_;
    COMPONENT_T* components_[5];
    TUNNEL_T tunnels_[4];
};

}
}
}
}

#endif
