#ifndef OPENAUTO_BLUETOOTHADAPTERMODEL_HPP
#define OPENAUTO_BLUETOOTHADAPTERMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/BluetoothAdapterModelItem.hpp>

namespace f1x::openauto::autoapp::UI {
  class BluetoothAdapterModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(BluetoothAdapterModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit BluetoothAdapterModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    BluetoothAdapterModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(BluetoothAdapterModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, QString value);

    QList<BluetoothAdapterModelItem *> m_comboBoxItems;
    BluetoothAdapterModelItem* m_currentComboBoxItem;

  };
}

#endif //OPENAUTO_BLUETOOTHADAPTERMODEL_HPP
