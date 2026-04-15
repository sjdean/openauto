#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceSinkVideo, "journeyos.service.mediasink.video")

namespace f1x::openauto::autoapp::service::mediasink {
  VideoService::VideoService(boost::asio::io_service &ioService,
                             aasdk::messenger::IMessenger::Pointer messenger,
                             projection::IVideoOutput::Pointer videoOutput)
      : VideoMediaSinkService(ioService,
                              std::make_shared<aasdk::channel::mediasink::video::channel::VideoChannel>(std::move(
                                                                                                            messenger)),
                              std::move(videoOutput)) {

  }
}



