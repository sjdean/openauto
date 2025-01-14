#include <f1x/openauto/autoapp/UI/FuelTypeModelItem.hpp>

namespace f1x::openauto::autoapp::UI {
FuelTypeModelItem::FuelTypeModelItem(QString display, aap_protobuf::service::sensorsource::message::FuelType value,
                                     QObject *parent) :
    QObject(parent),
    m_display(display),
    m_value(value) {

}

  QString FuelTypeModelItem::getDisplay() const { return m_display; }

  void FuelTypeModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  aap_protobuf::service::sensorsource::message::FuelType FuelTypeModelItem::getValue() const {
    return m_value;
  }

  void FuelTypeModelItem::setValue(const aap_protobuf::service::sensorsource::message::FuelType &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}