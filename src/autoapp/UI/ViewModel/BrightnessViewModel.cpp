#include "f1x/openauto/autoapp/UI/ViewModel/BrightnessViewModel.hpp"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
    BrightnessViewModel::BrightnessViewModel(configuration::IConfiguration::Pointer configuration,
                                             Controller::LightController &lightHandler,
                                             QObject *parent) : QObject(parent),
                                                                m_configuration(std::move(configuration)),
                                                                m_lightHandler(lightHandler) {
        QObject::connect(&m_lightHandler, &Controller::LightController::dayChanged,
                         this, &BrightnessViewModel::onLightChange);

        QObject::connect(&m_lightHandler, &Controller::LightController::lightsOnChanged,
                         this, &BrightnessViewModel::onLightChange);

        m_userBrightnessTarget = m_configuration->getSettingByName<int>("Screen", "Brightness");

        const int min = m_configuration->getSettingByName<int>(
    "Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMin" : "NightMin");
        const int max = m_configuration->getSettingByName<int>(
            "Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMax" : "NightMax");

        m_calculatedBrightness = calculateBrightness(min, max, m_userBrightnessTarget);

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
            qCInfo(lcVmBrightness) << "Backlight device:" << m_backlightPath
                                   << "max:" << m_backlightMaxBrightness;
        }
#endif
    }

    void BrightnessViewModel::onLightChange() {
        // The light state changed, so we must recalculate
        setTargetBrightness(m_userBrightnessTarget);
    }

    /**
     *
     * @param userBrightnessTarget Target brightness to set 0 to 255
     */
    void BrightnessViewModel::setTargetBrightness(const int userBrightnessTarget) {
        const int min = m_configuration->getSettingByName<int>(
            "Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMin" : "NightMin");
        const int max = m_configuration->getSettingByName<int>(
            "Screen", m_lightHandler.getDay() || m_lightHandler.getLightsOn() ? "DayMax" : "NightMax");

        const int calculatedBrightness = calculateBrightness(min, max, userBrightnessTarget);

        qCDebug(lcVmBrightness) << " Min " << min << " Max " << max << " Requested " << userBrightnessTarget <<
 " Calculated " << calculatedBrightness;

        if (m_calculatedBrightness != calculatedBrightness) {
            m_calculatedBrightness = calculatedBrightness;

#ifdef Q_OS_LINUX
            // Hardware Control via DBus (Non-blocking / Fire-and-forget)
            // We expect a custom service (running as root) to listen to this.
            QDBusMessage message = QDBusMessage::createMethodCall(
                "uk.co.cubeone.journeyos.backlight", // Service
                "/uk/co/cubeone/journeyos/backlight", // Path
                "uk.co.cubeone.journeyos.backlight", // Interface
                "SetBrightness" // Method
            );
            message << calculatedBrightness;

            // Send without blocking the UI thread
            QDBusConnection::systemBus().call(message, QDBus::NoBlock);
#endif
            emit screenBrightnessChanged();
        }
        if (m_userBrightnessTarget != userBrightnessTarget) {
            m_userBrightnessTarget = userBrightnessTarget;
            m_configuration->updateSettingByName("Screen", "Brightness", userBrightnessTarget);
            emit targetBrightnessChanged();
        }
    }

    /**
     * Get Brightness as set by User
     * @return brightness value to set 0 to 255
     */
    int BrightnessViewModel::getTargetBrightness() const {
        qInfo(lcVmBrightness) << "Target brightness: " << m_userBrightnessTarget;
        return m_userBrightnessTarget;
    }

    /**
     * Get Screen Brightness that has been pre-calculated
     * @return calculated brightness
     */
    int BrightnessViewModel::getScreenBrightness() const {
        qInfo(lcVmBrightness) << "Calculated brightness: " << m_calculatedBrightness;
        return m_calculatedBrightness;
    }

    /**
     *
     * @param min Minimum brightness bound
     * @param max Maximum brightness bound
     * @param target brightness
     * @return Brightness calculated within the minimum/maximum bound
     */
    int BrightnessViewModel::calculateBrightness(const int min, const int max, const int target) {
        int safeTarget = std::clamp(target, 0, 255);
        float ratio = safeTarget / 255.0f;

        return static_cast<int>(min + ((max - min) * ratio));
    }

    void BrightnessViewModel::saveSettings() const {
        m_configuration->save();
    }
}
