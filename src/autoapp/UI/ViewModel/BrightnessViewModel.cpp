#include "f1x/openauto/autoapp/UI/ViewModel/BrightnessViewModel.hpp"

#include <algorithm>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <utility>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcVmBrightness, "journeyos.brightness")

namespace f1x::openauto::autoapp::UI::ViewModel {
using configuration::ConfigGroup;
using configuration::ConfigKey;
    /**
     * Adjusts screen brightness or or backlight in response to light events
     * @param configuration A link to IConfiguration
     * @param lightHandler A link to the LightController object for time of day brightness calculations.
     * @param parent
     */
    BrightnessViewModel::BrightnessViewModel(configuration::IConfiguration::Pointer configuration,
                                             Controller::LightController &lightHandler,
                                             QObject *parent) : QObject(parent),
                                                                m_configuration(std::move(configuration)),
                                                                m_lightHandler(lightHandler) {
        QObject::connect(&m_lightHandler, &Controller::LightController::dayChanged,
                         this, &BrightnessViewModel::onLightChange);

        QObject::connect(&m_lightHandler, &Controller::LightController::lightsOnChanged,
                         this, &BrightnessViewModel::onLightChange);

        // Load saved brightness and clamp to current day/night [min,max] range
        const bool isDay = m_lightHandler.getDay() || m_lightHandler.getLightsOn();
        const int savedBrightness = m_configuration->getSettingByName<int>(ConfigGroup::Screen, ConfigKey::ScreenBrightness);
        const int min = m_configuration->getSettingByName<int>(ConfigGroup::Screen,
            isDay ? ConfigKey::ScreenDayMin : ConfigKey::ScreenNightMin);
        const int max = m_configuration->getSettingByName<int>(ConfigGroup::Screen,
            isDay ? ConfigKey::ScreenDayMax : ConfigKey::ScreenNightMax);
        m_userBrightnessTarget = std::clamp(savedBrightness, min, max);
        m_calculatedBrightness = m_userBrightnessTarget;

#ifdef Q_OS_LINUX
        // Detect sysfs backlight device, preferring the RPi official display driver
        static const QStringList preferred = {"rpi_backlight", "10-0045", "acpi_video0"};
        QDir backlightDir("/sys/class/backlight");
        for (const QString &name : preferred) {
            if (backlightDir.exists(name)) {
                m_backlightPath = "/sys/class/backlight/" + name;
                break;
            }
        }
        if (m_backlightPath.isEmpty()) {
            const QStringList entries = backlightDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (!entries.isEmpty())
                m_backlightPath = "/sys/class/backlight/" + entries.first();
        }
        if (!m_backlightPath.isEmpty()) {
            QFile maxFile(m_backlightPath + "/max_brightness");
            if (maxFile.open(QIODevice::ReadOnly)) {
                const int parsed = maxFile.readAll().trimmed().toInt();
                if (parsed > 0) m_backlightMaxBrightness = parsed;
            }
            qInfo(lcVmBrightness) << "backlight device=" << m_backlightPath
                                  << " max=" << m_backlightMaxBrightness;
        }
#endif
    }

    int BrightnessViewModel::getCurrentMin() const {
        const bool isDay = m_lightHandler.getDay() || m_lightHandler.getLightsOn();
        return m_configuration->getSettingByName<int>(ConfigGroup::Screen,
            isDay ? ConfigKey::ScreenDayMin : ConfigKey::ScreenNightMin);
    }

    int BrightnessViewModel::getCurrentMax() const {
        const bool isDay = m_lightHandler.getDay() || m_lightHandler.getLightsOn();
        return m_configuration->getSettingByName<int>(ConfigGroup::Screen,
            isDay ? ConfigKey::ScreenDayMax : ConfigKey::ScreenNightMax);
    }

    void BrightnessViewModel::onLightChange() {
        // Min/max bounds change when day/night switches — update slider bounds in QML
        emit currentMinChanged();
        emit currentMaxChanged();
        // Re-apply current target (clamps to new day/night range)
        setTargetBrightness(m_userBrightnessTarget);
    }

    /**
     * Set brightness directly in hardware units within [currentMin, currentMax].
     * @param userBrightnessTarget Hardware brightness value (not abstract 0-255 proportion)
     */
    void BrightnessViewModel::setTargetBrightness(const int userBrightnessTarget) {
        const int min = getCurrentMin();
        const int max = getCurrentMax();
        const int clamped = std::clamp(userBrightnessTarget, min, max);

        qDebug(lcVmBrightness) << "brightness min=" << min << " max=" << max
                               << " requested=" << userBrightnessTarget << " clamped=" << clamped;

        if (m_calculatedBrightness != clamped) {
            m_calculatedBrightness = clamped;

#ifdef Q_OS_LINUX
            // Hardware Control via DBus (Non-blocking / Fire-and-forget)
            QDBusMessage message = QDBusMessage::createMethodCall(
                "uk.co.cubeone.journeyos.backlight",
                "/uk/co/cubeone/journeyos/backlight",
                "uk.co.cubeone.journeyos.backlight",
                "SetBrightness"
            );
            message << clamped;
            QDBusConnection::systemBus().call(message, QDBus::NoBlock);
#endif
            emit screenBrightnessChanged();
        }
        if (m_userBrightnessTarget != clamped) {
            m_userBrightnessTarget = clamped;
            m_configuration->updateSettingByName(ConfigGroup::Screen, ConfigKey::ScreenBrightness, clamped);
            emit targetBrightnessChanged();
        }
    }

    /**
     * Get Brightness as set by User
     * @return brightness value to set 0 to 255
     */
    int BrightnessViewModel::getTargetBrightness() const {
        return m_userBrightnessTarget;
    }

    /**
     * Get Screen Brightness that has been pre-calculated
     * @return calculated brightness
     */
    int BrightnessViewModel::getScreenBrightness() const {
        return m_calculatedBrightness;
    }

    void BrightnessViewModel::saveSettings() const {
        m_configuration->save();
    }
}
