#ifndef OPENAUTO_ANDROIDAUTOMONITOR_HPP
#define OPENAUTO_ANDROIDAUTOMONITOR_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <f1x/openauto/autoapp/UI/IAndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::UI {

  class AndroidAutoConnectivityMethod : public QObject {
    Q_OBJECT

  public:
    enum Value {
      AA_INDETERMINATE,
      AA_USB,
      AA_WIFI
    };
    Q_ENUM(Value)
  };

  class AndroidAutoConnectivityState : public QObject {
  Q_OBJECT

  public:
    enum Value {
      AA_STARTUP,
      AA_DISCONNECTED,
      AA_CONNECTING,
      AA_CONNECTED
    };
    Q_ENUM(Value)
  };

  class AndroidAutoMonitor : public QObject, IAndroidAutoMonitor {
    Q_OBJECT
    Q_PROPERTY(AndroidAutoConnectivityState::Value state READ getState WRITE setState NOTIFY connectivityStateChanged)
    Q_PROPERTY(AndroidAutoConnectivityMethod::Value method READ getMethod WRITE setMethod NOTIFY connectivityMethodChanged)
  public:
    explicit AndroidAutoMonitor(QObject *parent = nullptr);
    void onConnectionMethodUpdate(AndroidAutoConnectivityMethod::Value method);
    void onConnectionStateUpdate(AndroidAutoConnectivityState::Value state);

    AndroidAutoConnectivityState::Value getState();
    AndroidAutoConnectivityMethod::Value getMethod();

    void setState(AndroidAutoConnectivityState::Value value);
    void setMethod(AndroidAutoConnectivityMethod::Value value);

  signals:
    void connectivityStateChanged(AndroidAutoConnectivityState::Value state);
    void connectivityMethodChanged(AndroidAutoConnectivityMethod::Value method);

  private:
    AndroidAutoConnectivityMethod::Value m_connectivityMethod;
    AndroidAutoConnectivityState::Value m_connectivityState;

  };
}
#endif //OPENAUTO_ANDROIDAUTOMONITOR_HPP