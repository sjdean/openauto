#include <f1x/openauto/autoapp/Service/MediaSink/MediaAudioService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkMedia, "journeyos.service.mediasink.media")

namespace f1x::openauto::autoapp::service::mediasink {
  MediaAudioService::MediaAudioService(boost::asio::io_service &ioService,
                                       aasdk::messenger::IMessenger::Pointer messenger,
                                       projection::IAudioOutput::Pointer audioOutput)
      : AudioMediaSinkService(ioService,
                              std::make_shared<aasdk::channel::mediasink::audio::channel::MediaAudioChannel>(strand_,
                                                                                                             std::move(
                                                                                                                 messenger)),
                              std::move(audioOutput)) {

  }
}



