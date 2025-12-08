#include <aasdk/Channel/MediaSink/Audio/Channel/TelephonyAudioChannel.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/TelephonyAudioService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkTelephony, "journeyos.service.mediasink.telephony")

namespace f1x::openauto::autoapp::service::mediasink {
  TelephonyAudioService::TelephonyAudioService(boost::asio::io_service &ioService,
                                               aasdk::messenger::IMessenger::Pointer messenger,
                                               projection::IAudioOutput::Pointer audioOutput)
      : AudioMediaSinkService(ioService,
                              std::make_shared<aasdk::channel::mediasink::audio::channel::TelephonyAudioChannel>(
                                  strand_,
                                  std::move(
                                      messenger)),
                              std::move(audioOutput)) {

  }
}



