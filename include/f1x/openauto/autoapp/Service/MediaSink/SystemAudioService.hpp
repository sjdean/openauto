#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/SystemAudioChannel.hpp>

namespace f1x::openauto::autoapp::service::mediasink {
    class SystemAudioService : public AudioMediaSinkService {
    public:
        SystemAudioService(aasdk::messenger::IMessenger::Pointer messenger,
                           projection::IAudioOutput::Pointer audioOutput);

    protected:
        projection::IAudioOutput::Pointer audioOutput_;
    };
}
