#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"

#include <qdatetime.h>
#include <utility>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcLight, "journeyos.light.controller")

namespace f1x::openauto::autoapp::UI::Controller {
using configuration::ConfigGroup;
using configuration::ConfigKey;
    /**
     * LightController - Simple wrapper to centralise information around the time of day,
     * or whether the car headlights are on and therefore whether we should dim the screen.
     *
     * @param configuration A shared pointer to an IConfiguration instance representing the
     * configuration settings for the LightController.
     * @return A new instance of the LightController class initialized with the provided configuration.
     */
    LightController::LightController(
        configuration::IConfiguration::Pointer configuration) : configuration_(
        std::move(configuration)), m_isDay(true), m_isLightsOn(false) {
    }

    bool LightController::getDay() const {
        const bool useClock = configuration_->getSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenUseClockDayNight);
        if (useClock) {
            //QDateTime local = QDateTime::currentDateTime();
            /** TODO - Establish Day/Night
             * Get Date/Time from Local Clock if Present, RTC, GPS, or Phone
             * Get Location from GPS or Phone or user defined location
             * If Internet Access is available, lookup Dawn/Dusk/Day/Night information
             * This is not the same as Light/Dark Mode - only for use in screen dimming
             */
        }
        return m_isDay;
    }

    bool LightController::getLightsOn() const {
        return m_isLightsOn;
    }

    void LightController::setDay(bool value) {
        if (m_isDay != value) {
            m_isDay = value;
            emit dayChanged();
        }
    }

    void LightController::setLightsOn(bool value) {
        if (m_isLightsOn != value) {
            m_isLightsOn = value;
            emit lightsOnChanged();
        }
    }

}
