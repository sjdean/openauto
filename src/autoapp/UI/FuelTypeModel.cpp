#include <f1x/openauto/autoapp/UI/FuelTypeModel.hpp>

namespace f1x::openauto::autoapp::UI {
  FuelTypeModel::FuelTypeModel(QObject *parent) : QObject(parent) {
    FuelTypeModel::populateComboBoxItems();
  }

  void FuelTypeModel::populateComboBoxItems() {
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

  QList<QObject *> FuelTypeModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (FuelTypeModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  FuelTypeModelItem* FuelTypeModel::getCurrentComboBoxItem() const { return m_currentComboBoxItem; }

  void FuelTypeModel::setCurrentComboBoxItem( FuelTypeModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void FuelTypeModel::addComboBoxItem(const QString &display,
                                      aap_protobuf::service::sensorsource::message::FuelType value) {
    auto item = FuelTypeModelItem(display, value);
    m_comboBoxItems.emplace_back(&item);
  }
}