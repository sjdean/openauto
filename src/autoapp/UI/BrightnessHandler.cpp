#include <f1x/openauto/autoapp/UI/BrightnessHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  BrightnessHandler::BrightnessHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration,  QObject *parent) :
      configuration_(configuration),
      QObject(parent) {

  }

  void BrightnessHandler::setBrightness(int brightness) {

    int min = configuration_->getSettingByName<int>("Audio", "audioPlaybackVolumeMin");
    int max = configuration_->getSettingByName<int>("Audio", "audioPlaybackVolumeMax");

    int calculatedBrightness = calculateBrightness(min, max, brightness);

    m_brightness = calculatedBrightness;
    emit brightnessChanged();
  }

  int BrightnessHandler::getBrightness() {
    return m_brightness;
  }

  int BrightnessHandler::calculateBrightness(int min, int max, int target) {
    auto calculatedVolume = min + (((max - min) * target) / 255);
    return calculatedVolume;
  }
}