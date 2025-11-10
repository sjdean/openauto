#include <f1x/openauto/autoapp/UI/Combo/PulseAudioDeviceModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Combo {
  PulseAudioDeviceModelItem::PulseAudioDeviceModelItem(QString display, QString value, QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }

  QString PulseAudioDeviceModelItem::getDisplay() const { return m_display; }

  void PulseAudioDeviceModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  QString PulseAudioDeviceModelItem::getValue() const {
    return m_value;
  }

  void PulseAudioDeviceModelItem::setValue(const QString &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}