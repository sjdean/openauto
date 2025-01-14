#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/VideoMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>


namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {
          class VideoService : public VideoMediaSinkService {
          public:
            VideoService(boost::asio::io_service &ioService,
                               aasdk::messenger::IMessenger::Pointer messenger,
                               projection::IVideoOutput::Pointer videoOutput);

          protected:
            projection::IVideoOutput::Pointer videoOutput;
          };

        }
      }
    }
  }
}