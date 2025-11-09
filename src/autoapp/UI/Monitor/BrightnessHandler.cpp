#include "f1x/openauto/autoapp/UI/Monitor/BrightnessHandler.hpp"

#include <utility>

namespace f1x::openauto::autoapp::UI::Monitor {
  /**
   * Adjusts screen brightness or or backlight in response to light events
   * @param configuration A link to IConfiguration
   * @param lightHandler A link to the LightHandler object for time of day brightness calculations.
   * @param parent
   */
  BrightnessHandler::BrightnessHandler(configuration::IConfiguration::Pointer configuration, LightHandler& lightHandler, QObject *parent) :
      QObject(parent),
      configuration_(std::move(configuration)),
      m_lightHandler(lightHandler) {
    QObject::connect(&m_lightHandler, &LightHandler::dayChanged,
                         this, &BrightnessHandler::onLightChange);

    QObject::connect(&m_lightHandler, &LightHandler::lightsOnChanged,
                     this, &BrightnessHandler::onLightChange);
  }

  void BrightnessHandler::onLightChange() {
    // The light state changed, so we must recalculate
    setBrightness(m_userBrightnessTarget);
  }

  /**
   *
   * @param userBrightnessTarget Target brightness to set 0 to 255
   */
  void BrightnessHandler::setBrightness(const int userBrightnessTarget) {
    const int min = configuration_->getSettingByName<int>("Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMin" : "NightMin");
    const int max = configuration_->getSettingByName<int>("Screen",  m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMax" : "NightMax");

    const int calculatedBrightness = calculateBrightness(min, max, userBrightnessTarget);

    m_userBrightnessTarget = userBrightnessTarget;
    m_calculatedBrightness = calculatedBrightness;
    configuration_->updateSettingByName("Screen", "Brightness", userBrightnessTarget);
    configuration_->updateSettingByName("Screen", "Brightness", calculatedBrightness);
    emit brightnessChanged();
  }

  /**
   * Get Brightness
   * @return brightness value to set 0 to 255
   */
  int BrightnessHandler::getBrightness() const {
    return m_userBrightnessTarget;
  }

  /**
   *
   * @param min Minimum brightness bound
   * @param max Maximum brightness bound
   * @param target brightness
   * @return Brightness calculated within the minimum/maximum bound
   */
  int BrightnessHandler::calculateBrightness(const int min, const int max, const int target) {
    const auto calculatedBrightness = min + (((max - min) * target) / 255);
    return calculatedBrightness;
  }
}