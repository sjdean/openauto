#ifndef OPENAUTO_RESOLUTIONMODEL_H
#define OPENAUTO_RESOLUTIONMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>

class ResolutionModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit ResolutionModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;
};

#endif //OPENAUTO_RESOLUTIONMODEL_H
