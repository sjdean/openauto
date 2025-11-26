#include "f1x/openauto/autoapp/UI/Monitor/LightHandler.hpp"
#include <utility>

namespace f1x::openauto::autoapp::UI::Monitor {
    /**
     * LightHandler - Simple wrapper to centralise information around the time of day,
     * or whether the car headlights are on and therefore whether we should dim the screen.
     *
     * @param configuration A shared pointer to an IConfiguration instance representing the
     * configuration settings for the LightHandler.
     * @return A new instance of the LightHandler class initialized with the provided configuration.
     */
    LightHandler::LightHandler(
        configuration::IConfiguration::Pointer configuration) : configuration_(
        std::move(configuration)), m_isDay(true), m_isLightsOn(false) {
    }

    bool LightHandler::getDay() const {
        // TODO: Get Time of Day as Necessary
        return m_isDay;
    }

    bool LightHandler::getLightsOn() const {
        return m_isLightsOn;
    }

    void LightHandler::setDay(bool value) {
        if (m_isDay != value) {
            m_isDay = value;
            emit dayChanged();
        }
    }

    void LightHandler::setLightsOn(bool value) {
        if (m_isLightsOn != value) {
            m_isLightsOn = value;
            emit lightsOnChanged();
        }
    }

}
