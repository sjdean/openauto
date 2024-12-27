//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/EvConnectorTypeModel.hpp>

EvConnectorTypeModel::EvConnectorTypeModel(QObject *parent) : ComboBoxModel(parent) {
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