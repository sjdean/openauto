#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace projection {

        VideoOutput::VideoOutput(configuration::IConfiguration::Pointer configuration)
            : configuration_(std::move(configuration)) {

        }

        aap_protobuf::service::media::sink::message::VideoFrameRateType VideoOutput::getVideoFPS() const {
          return configuration_->getVideoFPS();
        }

        aap_protobuf::service::media::sink::message::VideoCodecResolutionType VideoOutput::getVideoResolution() const {
          return configuration_->getVideoResolution();
        }

        size_t VideoOutput::getScreenDPI() const {
          return configuration_->getScreenDPI();
        }

        QRect VideoOutput::getVideoMargins() const {
          return configuration_->getVideoMargins();
        }

      }
    }
  }
}
