#include "f1x/openauto/autoapp/UI/Combo/BluetoothDeviceModel.hpp"

#include <f1x/openauto/autoapp/UI/BluetoothDeviceModel.hpp>
#include <QProcess>

namespace f1x::openauto::autoapp::UI {
  Combo::BluetoothDeviceModel::BluetoothDeviceModel(BluetoothHandler *bluetoothHandler, QObject *parent) : QObject(parent),
                                                                                                           m_bluetoothHandler(
                                                                                                             bluetoothHandler),
                                                                                                           m_currentComboBoxItem(
                                                                                                             nullptr) {
    BluetoothDeviceModel::populateComboBoxItems();
  }

  bool isBlueZRunning() {
    QProcess process;
    process.start("systemctl", QStringList() << "is-active" << "bluetooth.service");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    return output.trimmed() == "active";
  }

  /**
   * Populate list of Paired Bluetooth Devices
   */
  void BluetoothDeviceModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    if (isBlueZRunning()) {

      auto pairedDevices = m_bluetoothHandler->getPairedDeviceList();
      if (!pairedDevices.empty()) {
        for (auto pairedDevice: pairedDevices) {
          addComboBoxItem(pairedDevice.name, pairedDevice.address);
        }
      } else {
        addComboBoxItem("SettingsWindow", "none");
      }
    } else {
      addComboBoxItem("SettingsWindow", "none");
    }
  }

  QList<QObject *> BluetoothDeviceModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (BluetoothDeviceModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  BluetoothDeviceModelItem *BluetoothDeviceModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void BluetoothDeviceModel::setCurrentComboBoxItem(BluetoothDeviceModelItem *value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void BluetoothDeviceModel::addComboBoxItem(const QString &display,
                                             QString value) {
    auto item = new BluetoothDeviceModelItem(display, value);
    m_comboBoxItems.emplace_back(item);
  }
}