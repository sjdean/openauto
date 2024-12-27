#ifndef OPENAUTO_FRAMERATEMODEL_H
#define OPENAUTO_FRAMERATEMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>

class FrameRateModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit FrameRateModel(QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;


#endif //OPENAUTO_FRAMERATEMODEL_H
