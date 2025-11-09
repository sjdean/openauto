#include <f1x/openauto/autoapp/UI/DriverPositionModel.hpp>

namespace f1x::openauto::autoapp::UI {
  DriverPositionModel::DriverPositionModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
    DriverPositionModel::populateComboBoxItems();
  }

  void DriverPositionModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    addComboBoxItem("Left",
                    aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_LEFT);
    addComboBoxItem("Right",
                    aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
    addComboBoxItem("Center",
                    aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_CENTER);

  }

  QList<QObject *> DriverPositionModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (DriverPositionModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  DriverPositionModelItem* DriverPositionModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void DriverPositionModel::setCurrentComboBoxItem(DriverPositionModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void DriverPositionModel::addComboBoxItem(const QString &display,
                                            aap_protobuf::service::control::message::DriverPosition value) {
    auto item = new DriverPositionModelItem(display, value);
    m_comboBoxItems.emplace_back(item);
  }
}