#include <QBluetoothHostInfo>
#include <QBluetoothLocalDevice>
#include <f1x/openauto/autoapp/UI/BluetoothAdapterModel.hpp>
#include <QProcess>

namespace f1x::openauto::autoapp::UI {
  BluetoothAdapterModel::BluetoothAdapterModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
    BluetoothAdapterModel::populateComboBoxItems();
  }

  bool isBlueZRunning() {
    QProcess process;
    process.start("systemctl", QStringList() << "is-active" << "bluetooth.service");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    return output.trimmed() == "active";
  }

  void BluetoothAdapterModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    if (isBlueZRunning()) {

      QList<QBluetoothHostInfo> adapters = QBluetoothLocalDevice::allDevices();
      if (!adapters.isEmpty()) {
        for (const QBluetoothHostInfo &adapter: adapters) {
          QString adapterAddress = adapter.address().toString();

          addComboBoxItem(QString("%1 (%2)").arg(adapter.name()).arg(
                              adapterAddress).toUtf8().constData(),
                          adapterAddress);
        }
      } else {
        addComboBoxItem("SettingsWindow", "none");
      }
    } else {
      addComboBoxItem("SettingsWindow", "none");
    }
  }

  QList<QObject *> BluetoothAdapterModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (BluetoothAdapterModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  BluetoothAdapterModelItem* BluetoothAdapterModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void BluetoothAdapterModel::setCurrentComboBoxItem(BluetoothAdapterModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void BluetoothAdapterModel::addComboBoxItem(const QString &display,
                                        QString value) {
    auto item = new BluetoothAdapterModelItem(display, value);
    m_comboBoxItems.emplace_back(item);
  }
}