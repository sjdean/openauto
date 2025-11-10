#include <f1x/openauto/autoapp/UI/Combo/DriverPositionModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Combo {
  DriverPositionModelItem::DriverPositionModelItem(QString display,
                                                   aap_protobuf::service::control::message::DriverPosition value,
                                                   QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }

  QString DriverPositionModelItem::getDisplay() const { return m_display; }

  void DriverPositionModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  aap_protobuf::service::control::message::DriverPosition DriverPositionModelItem::getValue() const {
    return m_value;
  }

  void DriverPositionModelItem::setValue(const aap_protobuf::service::control::message::DriverPosition &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}