#ifndef OPENAUTO_DRIVERPOSITIONMODEL_HPP
#define OPENAUTO_DRIVERPOSITIONMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Combo/DriverPositionModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Combo {
  class DriverPositionModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(DriverPositionModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit DriverPositionModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    DriverPositionModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(DriverPositionModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, aap_protobuf::service::control::message::DriverPosition value);

  private:
    QList<DriverPositionModelItem *> m_comboBoxItems;
    DriverPositionModelItem* m_currentComboBoxItem;
  };
}
#endif //OPENAUTO_DRIVERPOSITIONMODEL_HPP
