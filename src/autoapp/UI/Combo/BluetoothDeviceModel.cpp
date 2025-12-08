#include "f1x/openauto/autoapp/UI/Combo/BluetoothDeviceModel.hpp"
#include <QProcess>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboBtDevice, "journeyos.bluetooth.client.list")

namespace f1x::openauto::autoapp::UI::Combo {
    BluetoothDeviceModel::BluetoothDeviceModel(Monitor::BluetoothHandler *bluetoothHandler,
                                               QObject *parent) : QObject(parent),
                                                                  m_bluetoothHandler(
                                                                      bluetoothHandler),
                                                                  m_currentComboBoxItem(
                                                                      nullptr) {
        populateComboBoxItems();
    }


    /**
     * Populate list of Paired Bluetooth Devices
     */
    void BluetoothDeviceModel::populateComboBoxItems() {
        m_comboBoxItems.clear();
        qDebug() << "Emptying Combo box...";

        auto pairedDevices = m_bluetoothHandler->getPairedDeviceList();
        qDebug() << "Queried QT Device List";
        if (!pairedDevices.empty()) {
            for (auto pairedDevice: pairedDevices) {
                QVariantMap properties = pairedDevice.toMap();
                addComboBoxItem(properties.value("name").toString(), properties.value("address").toString());
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
