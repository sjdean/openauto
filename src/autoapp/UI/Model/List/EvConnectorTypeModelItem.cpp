#include <f1x/openauto/autoapp/UI/Model/List/EvConnectorTypeModelItem.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboEvItem, "journeyos.evconnector.list.item")

namespace f1x::openauto::autoapp::UI::Model::List {
  EvConnectorTypeModelItem::EvConnectorTypeModelItem(QString display,
                                                     aap_protobuf::service::sensorsource::message::EvConnectorType value,
                                                     QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }

  QString EvConnectorTypeModelItem::getDisplay() const { return m_display; }

  void EvConnectorTypeModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  aap_protobuf::service::sensorsource::message::EvConnectorType EvConnectorTypeModelItem::getValue() const {
    return m_value;
  }

  void EvConnectorTypeModelItem::setValue(const aap_protobuf::service::sensorsource::message::EvConnectorType &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}