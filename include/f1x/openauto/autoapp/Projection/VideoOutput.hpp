#pragma once
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class VideoOutput: public IVideoOutput
{
public:
    VideoOutput(configuration::IConfiguration::Pointer configuration);

    aap_protobuf::service::media::sink::message::VideoFrameRateType getVideoFPS() const override;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() const override;
    size_t getScreenDPI() const override;
    QRect getVideoMargins() const override;

protected:
    configuration::IConfiguration::Pointer configuration_;
};

}
}
}
}
