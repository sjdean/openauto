#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/GuidanceAudioChannel.hpp>

namespace f1x::openauto::autoapp::service::mediasink {
    class GuidanceAudioService : public AudioMediaSinkService {
    public:
        GuidanceAudioService(boost::asio::io_service &ioService,
                             aasdk::messenger::IMessenger::Pointer messenger,
                             projection::IAudioOutput::Pointer audioOutput);
    };
}
