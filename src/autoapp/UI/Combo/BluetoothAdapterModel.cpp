#include "f1x/openauto/autoapp/UI/Combo/BluetoothAdapterModel.hpp"
#include <QBluetoothLocalDevice>
#include <QProcess>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboBtAdapter, "journeyos.bluetooth.adapter.list")


namespace f1x::openauto::autoapp::UI::Combo {
    // TODO: Bring in Bluetooth Monitor
    BluetoothAdapterModel::BluetoothAdapterModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
        populateComboBoxItems();
    }

    void BluetoothAdapterModel::populateComboBoxItems() {
        qDebug(lcComboBtAdapter) << "Clear existing list";
        m_comboBoxItems.clear();
        qDebug(lcComboBtAdapter) << "Get device list";
        QList<QBluetoothHostInfo> adapters = QBluetoothLocalDevice::allDevices();

        if (!adapters.isEmpty()) {
            qInfo(lcComboBtAdapter) << adapters.count() << " Bluetooth Adapters Found.";
            for (const QBluetoothHostInfo &adapter: adapters) {
                QString adapterAddress = adapter.address().toString();
                qDebug(lcComboBtAdapter) << "Processing adapter " << adapter.name();
                addComboBoxItem(QString("%1 (%2)").arg(adapter.name()).arg(
                                    adapterAddress).toUtf8().constData(),
                                adapterAddress);
            }
        } else {
            qInfo(lcComboBtAdapter) << "No Bluetooth Adapters Found.";
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

    BluetoothAdapterModelItem *BluetoothAdapterModel::getCurrentComboBoxItem() {
        if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
            m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
        }
        return m_currentComboBoxItem;
    }

    void BluetoothAdapterModel::setCurrentComboBoxItem(BluetoothAdapterModelItem *value) {
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
