#ifndef OPENAUTO_BLUETOOTHADAPTERMODEL_H
#define OPENAUTO_BLUETOOTHADAPTERMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"

class BluetoothAdapterModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit BluetoothAdapterModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;
};

#endif //OPENAUTO_BLUETOOTHADAPTERMODEL_H
