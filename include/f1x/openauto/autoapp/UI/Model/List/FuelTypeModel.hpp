#ifndef OPENAUTO_FUELTYPEMODEL_HPP
#define OPENAUTO_FUELTYPEMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Model/List/FuelTypeModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Model::List {
  class FuelTypeModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(FuelTypeModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit FuelTypeModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    FuelTypeModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(FuelTypeModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, aap_protobuf::service::sensorsource::message::FuelType value);
  private:
    QList<FuelTypeModelItem *> m_comboBoxItems;
    FuelTypeModelItem* m_currentComboBoxItem{};
  };
}
#endif //OPENAUTO_FUELTYPEMODEL_HPP
