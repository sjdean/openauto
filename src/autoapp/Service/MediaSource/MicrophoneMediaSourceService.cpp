#include <aasdk/Channel/MediaSource/Audio/MicrophoneAudioChannel.hpp>
#include <f1x/openauto/autoapp/Service/MediaSource/MicrophoneMediaSourceService.hpp>

namespace f1x::openauto::autoapp::service::mediasource {
  MicrophoneMediaSourceService::MicrophoneMediaSourceService(boost::asio::io_service &ioService,
                                                             aasdk::messenger::IMessenger::Pointer messenger,
                                                             projection::IAudioInput::Pointer audioOutput)
      : MediaSourceService(ioService,
                           std::make_shared<aasdk::channel::mediasource::audio::MicrophoneAudioChannel>(strand_,
                                                                                                        std::move(
                                                                                                            messenger)),
                           std::move(audioOutput)) {

  }
}



