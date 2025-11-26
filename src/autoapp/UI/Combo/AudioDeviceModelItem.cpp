#include <f1x/openauto/autoapp/UI/Combo/AudioDeviceModelItem.hpp> // Renamed

namespace f1x::openauto::autoapp::UI::Combo {
    // Renamed constructor
    AudioDeviceModelItem::AudioDeviceModelItem(QString display, QString value, QObject *parent) :
        QObject(parent),
        m_display(display),
        m_value(value) {

    }

    QString AudioDeviceModelItem::getDisplay() const { return m_display; }

    void AudioDeviceModelItem::setDisplay(const QString &display) {
        if (m_display != display) {
            m_display = display;

        }
    }

    QString AudioDeviceModelItem::getValue() const {
        return m_value;
    }

    void AudioDeviceModelItem::setValue(const QString &value) {
        if (m_value != value) { // Extract T from QVariant
            m_value = value;
        }
    }
}