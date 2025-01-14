#include <f1x/openauto/autoapp/UI/EvConnectorTypeModel.hpp>

namespace f1x::openauto::autoapp::UI {
  EvConnectorTypeModel::EvConnectorTypeModel(QObject *parent) : QObject(parent) {
    EvConnectorTypeModel::populateComboBoxItems();
  }

  void EvConnectorTypeModel::populateComboBoxItems() {
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

  EvConnectorTypeModelItem* EvConnectorTypeModel::getCurrentComboBoxItem() const { return m_currentComboBoxItem; }

  void EvConnectorTypeModel::setCurrentComboBoxItem(EvConnectorTypeModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void EvConnectorTypeModel::addComboBoxItem(const QString &display,
                                             aap_protobuf::service::sensorsource::message::EvConnectorType value) {
    auto item = EvConnectorTypeModelItem(display, value);
    m_comboBoxItems.emplace_back(&item);
  }
}