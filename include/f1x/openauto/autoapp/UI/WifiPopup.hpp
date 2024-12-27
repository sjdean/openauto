#ifndef OPENAUTO_WIFIPOPUP_H
#define OPENAUTO_WIFIPOPUP_H

#include <QtCore/QObject>

class WifiPopup : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString ssid READ ssid WRITE setSsid NOTIFY ssidChanged)
  Q_PROPERTY(int wifiStatus READ wifiStatus WRITE setWifiStatus NOTIFY wifiStatusChanged)

public:
  explicit WifiPopup(QObject *parent = nullptr);

signals:
    void ssidChanged();
    void wifiStatusChanged();

private:
  QString setSsid();
  void setSsid(QString value);

  int wifiStatus();
  void wifiStatusChanged(int value);
};

#endif //OPENAUTO_WIFIPOPUP_H