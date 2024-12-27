#ifndef OPENAUTO_EVCONNECTORTYPEMODEL_H
#define OPENAUTO_EVCONNECTORTYPEMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>

class EvConnectorTypeModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit EvConnectorTypeModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;
};

#endif //OPENAUTO_EVCONNECTORTYPEMODEL_H
