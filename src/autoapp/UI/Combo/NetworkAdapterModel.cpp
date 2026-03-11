#include <QNetworkInterface>
#include "f1x/openauto/autoapp/UI/Combo/NetworkAdapterModel.hpp"


namespace f1x::openauto::autoapp::UI::Combo {

    NetworkAdapterModel::NetworkAdapterModel(QObject *parent) : QObject(parent) {
        populateComboBoxItems();

    }

    void NetworkAdapterModel::populateComboBoxItems() {
        m_comboBoxItems.clear();
        addComboBoxItem("None", "No Device Selected", "00:00:00:00:00:00"); // Default
        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : interfaces) {
            // Basic filtering to remove loopback or obviously inactive ones
            if (interface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;

            QString display = QString("%1 (%2)").arg(interface.humanReadableName(), interface.hardwareAddress());
            if (interface.hardwareAddress().isEmpty()) {
                display = interface.humanReadableName();
            }
            addComboBoxItem(display, interface.name(), interface.hardwareAddress());
        }
    }

    QList<QObject *> NetworkAdapterModel::getComboBoxItems() const {
        QList<QObject *> list;
        for (NetworkAdapterModelItem *item: m_comboBoxItems) { // Use renamed item
            list.append(item);
        }
        return list;
    }

    NetworkAdapterModelItem* NetworkAdapterModel::getCurrentComboBoxItem() { // Use renamed item
        if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
            m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
        }
        return m_currentComboBoxItem;
    }

    void NetworkAdapterModel::setCurrentComboBoxItem(NetworkAdapterModelItem* value) { // Use renamed item
        if (m_currentComboBoxItem != value) {
            m_currentComboBoxItem = value;
            emit currentComboBoxItemChanged();
        }
    }

    void NetworkAdapterModel::addComboBoxItem(const QString &display, const QString &interfaceName, const QString &hardwareAddress) {
        auto item = new NetworkAdapterModelItem(display, interfaceName, hardwareAddress, this);
        m_comboBoxItems.emplace_back(item);
        emit comboBoxItemsChanged();
    }
}