#include "f1x/openauto/autoapp/UI/Model/List/EvConnectorTypeModel.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboEv, "journeyos.evconnector.list")

namespace f1x::openauto::autoapp::UI::Model::List {
    EvConnectorTypeModel::EvConnectorTypeModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
      populateComboBoxItems();
  }

  void EvConnectorTypeModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    addComboBoxItem("Unknown",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_UNKNOWN);
    addComboBoxItem("J1772",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_J1772);
    addComboBoxItem("Mannekes",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_MENNEKES);
    addComboBoxItem("Chademo",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_CHADEMO);
    addComboBoxItem("Combo 1",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_COMBO_1);
    addComboBoxItem("Combo 2",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_COMBO_2);
    addComboBoxItem("Tesla Supercharger",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_TESLA_SUPERCHARGER);
    addComboBoxItem("GBT",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_GBT);
    addComboBoxItem("Other",
                    aap_protobuf::service::sensorsource::message::EvConnectorType::EV_CONNECTOR_TYPE_OTHER);

  }

  QList<QObject *> EvConnectorTypeModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (EvConnectorTypeModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  EvConnectorTypeModelItem* EvConnectorTypeModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void EvConnectorTypeModel::setCurrentComboBoxItem(EvConnectorTypeModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void EvConnectorTypeModel::addComboBoxItem(const QString &display,
                                             aap_protobuf::service::sensorsource::message::EvConnectorType value) {
    auto item = new EvConnectorTypeModelItem(display, value);
    m_comboBoxItems.emplace_back(item);
  }
}
