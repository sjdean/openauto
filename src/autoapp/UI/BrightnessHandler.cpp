#include <f1x/openauto/autoapp/UI/BrightnessHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  BrightnessHandler::BrightnessHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration,  QObject *parent) :
      QObject(parent),
      configuration_(configuration) {

  }

  void BrightnessHandler::setBrightness(int brightness) {
    bool isNight = false;
    // TODO: Add Day/Night Handler, so if Day we get one value, if night we get another
    int min = configuration_->getSettingByName<int>("Screen", isNight ? "NightMin" : "DayMin");
    int max = configuration_->getSettingByName<int>("Screen", isNight ? "NightMax" : "DayMax");

    int calculatedBrightness = calculateBrightness(min, max, brightness);

    m_brightness = calculatedBrightness;
    emit brightnessChanged();
  }

  int BrightnessHandler::getBrightness() {
    return m_brightness;
  }

  int BrightnessHandler::calculateBrightness(int min, int max, int target) {
    auto calculatedBrightness = min + (((max - min) * target) / 255);
    return calculatedBrightness;
  }
}