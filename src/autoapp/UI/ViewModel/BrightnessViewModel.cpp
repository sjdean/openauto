#include "f1x/openauto/autoapp/UI/ViewModel/BrightnessViewModel.hpp"

#include <utility>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVmBrightness, "journeyos.brightness")

namespace f1x::openauto::autoapp::UI::ViewModel {
  /**
   * Adjusts screen brightness or or backlight in response to light events
   * @param configuration A link to IConfiguration
   * @param lightHandler A link to the LightController object for time of day brightness calculations.
   * @param parent
   */
  BrightnessViewModel::BrightnessViewModel(configuration::IConfiguration::Pointer configuration, Controller::LightController& lightHandler, QObject *parent) :
      QObject(parent),
      m_configuration(std::move(configuration)),
      m_lightHandler(lightHandler) {
    QObject::connect(&m_lightHandler, &Controller::LightController::dayChanged,
                         this, &BrightnessViewModel::onLightChange);

    QObject::connect(&m_lightHandler, &Controller::LightController::lightsOnChanged,
                     this, &BrightnessViewModel::onLightChange);
  }

  void BrightnessViewModel::onLightChange() {
    // The light state changed, so we must recalculate
    setBrightness(m_userBrightnessTarget);
  }

  /**
   *
   * @param userBrightnessTarget Target brightness to set 0 to 255
   */
  void BrightnessViewModel::setBrightness(const int userBrightnessTarget) {
    const int min = m_configuration->getSettingByName<int>("Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMin" : "NightMin");
    const int max = m_configuration->getSettingByName<int>("Screen",  m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMax" : "NightMax");

    const int calculatedBrightness = calculateBrightness(min, max, userBrightnessTarget);

    m_userBrightnessTarget = userBrightnessTarget;
    m_calculatedBrightness = calculatedBrightness;
    m_configuration->updateSettingByName("Screen", "Brightness", userBrightnessTarget);
    m_configuration->updateSettingByName("Screen", "Brightness", calculatedBrightness);
    emit brightnessChanged();
  }

  /**
   * Get Brightness
   * @return brightness value to set 0 to 255
   */
  int BrightnessViewModel::getBrightness() const {
    return m_userBrightnessTarget;
  }

  /**
   *
   * @param min Minimum brightness bound
   * @param max Maximum brightness bound
   * @param target brightness
   * @return Brightness calculated within the minimum/maximum bound
   */
  int BrightnessViewModel::calculateBrightness(const int min, const int max, const int target) {
    const auto calculatedBrightness = min + (((max - min) * target) / 255);
    return calculatedBrightness;
  }
}