#ifndef OPENAUTO_EVCONNECTORTYPEMODEL_HPP
#define OPENAUTO_EVCONNECTORTYPEMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Model/List/EvConnectorTypeModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Model::List {
  class EvConnectorTypeModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(EvConnectorTypeModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit EvConnectorTypeModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    EvConnectorTypeModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(EvConnectorTypeModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, aap_protobuf::service::sensorsource::message::EvConnectorType value);
  private:
    QList<EvConnectorTypeModelItem*> m_comboBoxItems;
    EvConnectorTypeModelItem* m_currentComboBoxItem;
  };
}
#endif //OPENAUTO_EVCONNECTORTYPEMODEL_HPP
