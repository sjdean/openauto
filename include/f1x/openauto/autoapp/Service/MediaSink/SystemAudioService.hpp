

#pragma once#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/SystemAudioChannel.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {
          class SystemAudioService : public AudioMediaSinkService {
          public:
            SystemAudioService(boost::asio::io_service &ioService,
                               aasdk::messenger::IMessenger::Pointer messenger,
                               projection::IAudioOutput::Pointer audioOutput);

          protected:
            projection::IAudioOutput::Pointer audioOutput_;
          };

        }
      }
    }
  }
}