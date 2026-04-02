#ifndef OPENAUTO_BLUETOOTHDEVICEMODEL_HPP
#define OPENAUTO_BLUETOOTHDEVICEMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Model/List/BluetoothDeviceModelItem.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/LinuxBluetoothManager.hpp>

namespace f1x::openauto::autoapp::UI::Model::List {
  class BluetoothDeviceModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(BluetoothDeviceModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit BluetoothDeviceModel(Monitor::LinuxBluetoothManager *bluetoothHandler, QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    BluetoothDeviceModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(BluetoothDeviceModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, QString value);

  private:
    Monitor::LinuxBluetoothManager *m_bluetoothHandler;
    QList<BluetoothDeviceModelItem *> m_comboBoxItems;
    BluetoothDeviceModelItem* m_currentComboBoxItem;

  };
}

#endif //OPENAUTO_BLUETOOTHDEVICEMODEL_HPP
