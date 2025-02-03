#include <f1x/openauto/autoapp/UI/BrightnessHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  /**
   * Adjusts screen brightness or or backlight in response to light events
   * @param configuration A link to IConfiguration
   * @param lightHandler A link to the LightHandler object for time of day brightness calculations.
   * @param parent
   */
  BrightnessHandler::BrightnessHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, f1x::openauto::autoapp::UI::LightHandler lightHandler, QObject *parent) :
      QObject(parent),
      configuration_(configuration),
      m_lightHandler(lightHandler) {

  }

  /**
   *
   * @param brightness Target brightness to set 0 to 255
   */
  void BrightnessHandler::setBrightness(int brightness) {
    bool isNight = false;
    int min = configuration_->getSettingByName<int>("Screen", m_lightHandler.getIsDay() ? "DayMin" : "NightMin");
    int max = configuration_->getSettingByName<int>("Screen",  m_lightHandler.getIsDay() ? "DayMax" : "NightMax");

    int calculatedBrightness = calculateBrightness(min, max, brightness);

    m_brightness = calculatedBrightness;
    configuration_->updateSettingByName("Screen", "Brightness", calculatedBrightness);
    emit brightnessChanged();
  }

  /**
   * Get Brightness
   * @return brightness value to set 0 to 255
   */
  int BrightnessHandler::getBrightness() {
    return m_brightness;
  }

  /**
   *
   * @param min Minimum brightness bound
   * @param max Maximum brightness bound
   * @param target brightness
   * @return Brightness calculated within the minimum/maximum bound
   */
  int BrightnessHandler::calculateBrightness(int min, int max, int target) {
    auto calculatedBrightness = min + (((max - min) * target) / 255);
    return calculatedBrightness;
  }
}