#ifndef OPENAUTO_BRIGHTNESSPOPUP_HPP
#define OPENAUTO_BRIGHTNESSPOPUP_HPP

#include <QtCore/QObject>

class BrightnessPopup : public QObject {
  Q_OBJECT
  Q_PROPERTY(int brightnessValue READ brightnessValue WRITE setBrightnessValue NOTIFY brightnessValueChanged)

public:
  explicit BrightnessPopup(QObject *parent = nullptr);

  signals:
  void brightnessValueChanged();

private:
  int brightnessValue();
  void setBrightnessValue(int value);
};

#endif //OPENAUTO_BRIGHTNESSPOPUP_HPP
