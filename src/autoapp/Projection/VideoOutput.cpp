#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVideoOut, "journeyos.video.output")
namespace f1x::openauto::autoapp::projection {

  VideoOutput::VideoOutput(configuration::IConfiguration::Pointer configuration)
      : configuration_(std::move(configuration)) {

  }

  aap_protobuf::service::media::sink::message::VideoFrameRateType VideoOutput::getVideoFPS() const {
    return static_cast<aap_protobuf::service::media::sink::message::VideoFrameRateType>(configuration_->getSettingByName<int>(
        "AndroidAuto", "FrameRate"));
  }

  aap_protobuf::service::media::sink::message::VideoCodecResolutionType VideoOutput::getVideoResolution() const {
    return static_cast<aap_protobuf::service::media::sink::message::VideoCodecResolutionType>(configuration_->getSettingByName<int>(
        "AndroidAuto", "Resolution"));
  }

  size_t VideoOutput::getScreenDPI() const {
    return configuration_->getSettingByName<int>("Screen", "DPI");
  }

  QRect VideoOutput::getVideoMargins() const {
    QRect videoMargins(0, 0, configuration_->getSettingByName<int>("Video", "Width"), configuration_->getSettingByName<int>("Video", "Height"));
    return videoMargins;
  }
}



