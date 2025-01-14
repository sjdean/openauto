#include <f1x/openauto/autoapp/Service/MediaSink/GuidanceAudioService.hpp>

namespace f1x::openauto::autoapp::service::mediasink {
  using f1x::openauto::autoapp::service::mediasink::GuidanceAudioService;
  using f1x::openauto::autoapp::service::mediasink::AudioMediaSinkService;
  using aasdk::channel::mediasink::audio::channel::GuidanceAudioChannel;

  GuidanceAudioService::GuidanceAudioService(boost::asio::io_service &ioService,
                                             aasdk::messenger::IMessenger::Pointer messenger,
                                             projection::IAudioOutput::Pointer audioOutput)
      : AudioMediaSinkService(
      ioService,
      std::make_shared<GuidanceAudioChannel>(strand_, std::move(messenger)),
      std::move(audioOutput)) {

  }
}



