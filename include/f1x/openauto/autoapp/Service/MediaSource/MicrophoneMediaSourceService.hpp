#pragma once
#include <f1x/openauto/autoapp/Service/MediaSource/MediaSourceService.hpp>
#include <aasdk/Channel/MediaSource/Audio/MicrophoneAudioChannel.hpp>


namespace f1x::openauto::autoapp::service::mediasource {
    class MicrophoneMediaSourceService : public MediaSourceService {
    public:
        MicrophoneMediaSourceService(aasdk::messenger::IMessenger::Pointer messenger,
                                     projection::IAudioInput::Pointer audioInput);

    protected:
        projection::IAudioInput::Pointer audioInput_;
    };
}
