#include <f1x/openauto/autoapp/UI/ComboBoxModel.hpp>

ComboBoxModel::ComboBoxModel(QObject *parent) : ComboBoxModel(parent) {
  ComboBoxModel::populateComboBoxItems();
}

QList<QObject *> ComboBoxModel::comboBoxItems() const {
  QList<QObject *> list;
  for (ComboBoxItem *item: m_comboBoxItems) {
    list.append(item);
  }
  return list;
}

int ComboBoxModel::currentComboBoxItem() const { return m_currentComboBoxItem; }
void ComboBoxModel::setCurrentComboBoxItem(int value) {
  if (m_currentComboBoxItem != value) {
    m_currentComboBoxItem = value;
    emit currentComboBoxItemChanged();
  }
}



