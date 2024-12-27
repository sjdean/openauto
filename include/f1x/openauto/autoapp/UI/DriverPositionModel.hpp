#ifndef OPENAUTO_DRIVERPOSITIONMODEL_H
#define OPENAUTO_DRIVERPOSITIONMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>

class DriverPositionModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit DriverPositionModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;
};

#endif //OPENAUTO_DRIVERPOSITIONMODEL_H
