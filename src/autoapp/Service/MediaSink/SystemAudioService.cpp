#include <aasdk/Channel/MediaSink/Audio/Channel/SystemAudioChannel.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/SystemAudioService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkSystem, "journeyos.service.mediasink.system")

namespace f1x::openauto::autoapp::service::mediasink {
  SystemAudioService::SystemAudioService(boost::asio::io_service &ioService,
                                         aasdk::messenger::IMessenger::Pointer messenger,
                                         projection::IAudioOutput::Pointer audioOutput)
      : AudioMediaSinkService(ioService,
                              std::make_shared<aasdk::channel::mediasink::audio::channel::SystemAudioChannel>(strand_,
                                                                                                              std::move(
                                                                                                                  messenger)),
                              std::move(audioOutput)) {

  }

}



