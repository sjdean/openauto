#ifndef OPENAUTO_NETWORKADAPTERMODEL_HPP
#define OPENAUTO_NETWORKADAPTERMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Combo/NetworkAdapterModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Combo {
    class NetworkAdapterModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(QList<QObject*> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)

    public:
        explicit NetworkAdapterModel(QObject *parent = nullptr);
        QList<QObject*> getComboBoxItems() const;
        Q_INVOKABLE int indexOf(const QString &interfaceName);

        signals:
            void comboBoxItemsChanged();

    private:
        void populate();
        QList<NetworkAdapterModelItem*> m_items;
    };
}

#endif //OPENAUTO_NETWORKADAPTERMODEL_HPP
