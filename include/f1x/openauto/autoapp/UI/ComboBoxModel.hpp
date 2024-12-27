#ifndef OPENAUTO_COMBOBOXMODEL_H
#define OPENAUTO_COMBOBOXMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxItem.hpp"

class ComboBoxModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<QObject *> comboBoxItems READ comboBoxItems NOTIFY comboBoxItemsChanged)
  Q_PROPERTY(int currentComboBoxItem READ currentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

public:
  explicit ComboBoxModel(QObject *parent = nullptr);
  virtual ~ComboBoxModel() = default;

signals:
  void comboBoxItemsChanged();
  void currentComboBoxItemChanged();

protected:
  virtual void populateComboBoxItems();
  void addComboBoxItem(const QString &display, int value);
  int currentComboBoxItem() const;
  QList<QObject *> comboBoxItems() const;
  void setCurrentComboBoxItem(int value);

private:
  QList<ComboBoxItem*> m_comboBoxItems;
  int m_currentComboBoxItem{};


};

#endif //OPENAUTO_COMBOBOXMODEL_H
