#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoService.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {
          VideoService::VideoService(boost::asio::io_service &ioService,
                                               aasdk::messenger::IMessenger::Pointer messenger,
                                               projection::IVideoOutput::Pointer videoOutput)
              : VideoMediaSinkService(ioService, std::make_shared<aasdk::channel::mediasink::video::channel::VideoChannel>(strand_,
                                                                                                                       std::move(
                                                                                                                           messenger)),
                                      std::move(videoOutput)) {

          }
        }
      }
    }
  }
}