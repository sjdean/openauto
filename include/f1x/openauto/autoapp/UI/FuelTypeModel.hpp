#ifndef OPENAUTO_FUELTYPEMODEL_H
#define OPENAUTO_FUELTYPEMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include "ComboBoxModel.hpp"

class FuelTypeModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit FuelTypeModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;


#endif //OPENAUTO_FUELTYPEMODEL_H
