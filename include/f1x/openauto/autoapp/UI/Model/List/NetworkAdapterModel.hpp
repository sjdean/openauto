#ifndef OPENAUTO_NETWORKADAPTERMODEL_HPP
#define OPENAUTO_NETWORKADAPTERMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Model/List/NetworkAdapterModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Model::List {
    class NetworkAdapterModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(QList<QObject*> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
        Q_PROPERTY(NetworkAdapterModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

    public:
        explicit NetworkAdapterModel(QObject *parent = nullptr);

        void addComboBoxItem(const QString &display, const QString &interfaceName, const QString &hardwareAddress);

    signals:
        void comboBoxItemsChanged();
        void currentComboBoxItemChanged();

    protected:
        QList<QObject *> getComboBoxItems() const;
        NetworkAdapterModelItem* getCurrentComboBoxItem();
        void setCurrentComboBoxItem(NetworkAdapterModelItem* value);

    private:
        void populateComboBoxItems();
        QList<NetworkAdapterModelItem *> m_comboBoxItems;
        NetworkAdapterModelItem* m_currentComboBoxItem;
    };
}

#endif //OPENAUTO_NETWORKADAPTERMODEL_HPP
