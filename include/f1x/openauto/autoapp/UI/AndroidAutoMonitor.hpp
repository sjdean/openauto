#ifndef OPENAUTO_ANDROIDAUTOMONITOR_HPP
#define OPENAUTO_ANDROIDAUTOMONITOR_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <f1x/openauto/autoapp/UI/IAndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::UI {
  enum AndroidAutoConnectivityState {
    AA_STARTUP,
    AA_DISCONNECTED,
    AA_CONNECTING,
    AA_CONNECTED
  };

  enum AndroidAutoConnectivityMethod {
    AA_INDETERMINATE,
    AA_USB,
    AA_WIFI
  };

  class AndroidAutoMonitor : public QObject, IAndroidAutoMonitor {
    Q_OBJECT
    Q_ENUM(AndroidAutoConnectivityState)
    Q_ENUM(AndroidAutoConnectivityMethod)
  public:
    explicit AndroidAutoMonitor(QObject *parent = nullptr);
    void onConnectionMethodUpdate(AndroidAutoConnectivityMethod method);
    void onConnectionStateUpdate(AndroidAutoConnectivityState state);

  signals:

    void connectivityStateChanged(AndroidAutoConnectivityState state);
    void connectivityMethodChanged(AndroidAutoConnectivityMethod method);

  private:
    AndroidAutoConnectivityMethod m_connectivityMethod;
    AndroidAutoConnectivityState m_connectivityState;

  };
}
#endif //OPENAUTO_ANDROIDAUTOMONITOR_HPP