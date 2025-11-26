#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/TelephonyAudioChannel.hpp>


namespace f1x::openauto::autoapp::service::mediasink {
    class TelephonyAudioService : public AudioMediaSinkService {
    public:
        TelephonyAudioService(boost::asio::io_service &ioService,
                              aasdk::messenger::IMessenger::Pointer messenger,
                              projection::IAudioOutput::Pointer audioOutput);

    protected:
        projection::IAudioOutput::Pointer audioOutput_;
    };
}
