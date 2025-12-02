#include <QNetworkInterface>
#include "f1x/openauto/autoapp/UI/Combo/NetworkAdapterModel.hpp"

namespace f1x::openauto::autoapp::UI::Combo {

    NetworkAdapterModel::NetworkAdapterModel(QObject *parent) : QObject(parent) {
        populate();
    }

    void NetworkAdapterModel::populate() {
        qDeleteAll(m_items);
        m_items.clear();

        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : interfaces) {
            // Basic filtering to remove loopback or obviously inactive ones
            if (interface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;

            QString display = QString("%1 (%2)").arg(interface.humanReadableName(), interface.hardwareAddress());
            if (interface.hardwareAddress().isEmpty()) {
                display = interface.humanReadableName();
            }

            auto item = new NetworkAdapterModelItem(display, interface.name(), interface.hardwareAddress(), this);
            m_items.append(item);
        }
        emit comboBoxItemsChanged();
    }

    QList<QObject*> NetworkAdapterModel::getComboBoxItems() const {
        QList<QObject*> list;
        for (auto *item : m_items) {
            list.append(item);
        }
        return list;
    }

    int NetworkAdapterModel::indexOf(const QString &interfaceName) {
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items[i]->getInterfaceName() == interfaceName) return i;
        }
        return -1;
    }

}