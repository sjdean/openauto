#ifndef OPENAUTO_WIFIPOPUP_H
#define OPENAUTO_WIFIPOPUP_H

#include <QtCore/QObject>
namespace f1x::openauto::autoapp::UI {
  class WifiPopup : public QObject {
  Q_OBJECT

    Q_PROPERTY(QString ssid READ getSsid WRITE setSsid NOTIFY ssidChanged)
    Q_PROPERTY(int wifiStatus READ getWifiStatus WRITE setWifiStatus NOTIFY wifiStatusChanged)

  public:
    explicit WifiPopup(QObject *parent = nullptr);

  signals:

    void ssidChanged();

    void wifiStatusChanged();

  private:
    QString getSsid();

    void setSsid(QString value);

    int getWifiStatus();

    void setWifiStatus(int value);

    int m_wifiStatus;
    QString m_ssid;
  };
}

#endif //OPENAUTO_WIFIPOPUP_H