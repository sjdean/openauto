//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/DriverPositionModel.hpp>

DriverPositionModel::DriverPositionModel(QObject *parent) : ComboBoxModel(parent) {
  DriverPositionModel::populateComboBoxItems();
}

void DriverPositionModel::populateComboBoxItems() {
  addComboBoxItem("Left",
                  aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_LEFT);
  addComboBoxItem("Right",
                  aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
  addComboBoxItem("Center",
                  aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_CENTER);

}

