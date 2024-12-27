#include <QBluetoothHostInfo>
#include <QBluetoothLocalDevice>

BluetoothAdapterModel::BluetoothAdapterModel(QObject *parent) : ComboBoxModel(parent) {
  BluetoothAdapterModel::populateComboBoxItems();
}

void BluetoothAdapterModel::populateComboBoxItems() {
  QList<QBluetoothHostInfo> adapters = QBluetoothLocalDevice::allDevices();
  if (!adapters.isEmpty()) {
    for (const QBluetoothHostInfo &adapter: adapters) {
      QString adapterAddress = adapter.address().toString();

      addComboBoxItem(QString("%1 (%2)").arg(adapter.name()).arg(
          adapterAddress).toUtf8().constData(), QVariant(adapterAddress));
    }
  } else {
    addComboBoxItem("SettingsWindow", "none");
  }
}