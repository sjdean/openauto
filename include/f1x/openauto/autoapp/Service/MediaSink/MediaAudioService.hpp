#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/MediaAudioChannel.hpp>


namespace f1x::openauto::autoapp::service::mediasink {
    class MediaAudioService : public AudioMediaSinkService {
    public:
        MediaAudioService(boost::asio::io_service &ioService,
                          aasdk::messenger::IMessenger::Pointer messenger,
                          projection::IAudioOutput::Pointer audioOutput);
    };
}
