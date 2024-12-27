//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/FuelTypeModel.hpp>

FuelTypeModel::FuelTypeModel(QObject *parent) : ComboBoxModel(parent) {
  FuelTypeModel::populateComboBoxItems();
}

void FuelTypeModel::populateComboBoxItems {
    addComboBoxItem("BioDiesel",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_BIODIESEL);
    addComboBoxItem("Diesel",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_DIESEL_1);
    addComboBoxItem("E85",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_E85);
    addComboBoxItem("CNG",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_CNG);
    addComboBoxItem("Electric",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_ELECTRIC);
    addComboBoxItem("Hydrogen",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_HYDROGEN);
    addComboBoxItem("Leaded",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_LEADED);
    addComboBoxItem("Unleaded",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_UNLEADED);
    addComboBoxItem("LNG",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_LNG);
    addComboBoxItem("LPG",
              aap_protobuf::service::sensorsource::message::FuelType::FUEL_TYPE_LPG);
}
