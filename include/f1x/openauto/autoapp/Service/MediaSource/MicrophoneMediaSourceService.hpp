

#pragma once#include <f1x/openauto/autoapp/Service/MediaSource/MediaSourceService.hpp>
#include <aasdk/Channel/MediaSource/Audio/MicrophoneAudioChannel.hpp>


namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasource {
          class MicrophoneMediaSourceService : public MediaSourceService {
          public:
            MicrophoneMediaSourceService(boost::asio::io_service &ioService,
                                  aasdk::messenger::IMessenger::Pointer messenger,
                                  projection::IAudioInput::Pointer audioInput);

          protected:
            projection::IAudioInput::Pointer audioInput_;
          };

        }
      }
    }
  }
}