#ifndef OPENAUTO_VOLUMEPOPUP_H
#define OPENAUTO_VOLUMEPOPUP_H

#include <QtCore/QObject>
#include <string>
#include <cstdio>

class VolumePopup : public QObject {
  Q_OBJECT
  Q_PROPERTY(int volumeValue READ volumeValue WRITE setVolumeValue NOTIFY volumeValueChanged)

public:
  explicit VolumePopup(QObject *parent = nullptr);

signals:
  void volumeValueChanged();

private:
  int volumeValue();
  void setVolumeValue(int value);
};

#endif //OPENAUTO_VOLUMEPOPUP_H
