#ifndef OPENAUTO_ANDROIDAUTOMONITOR_HPP
#define OPENAUTO_ANDROIDAUTOMONITOR_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <f1x/openauto/autoapp/UI/Monitor/IAndroidAutoMonitor.hpp>
#include <f1x/openauto/autoapp/UI/Enum/AndroidAutoConnectivityMethod.hpp>
#include <f1x/openauto/autoapp/UI/Enum/AndroidAutoConnectivityState.hpp>

namespace f1x::openauto::autoapp::UI::Monitor {

  class AndroidAutoMonitor : public QObject, IAndroidAutoMonitor {
    Q_OBJECT
    Q_PROPERTY(Enum::AndroidAutoConnectivityState::Value state READ getState WRITE setState NOTIFY connectivityStateChanged)
    Q_PROPERTY(Enum::AndroidAutoConnectivityMethod::Value method READ getMethod WRITE setMethod NOTIFY connectivityMethodChanged)
  public:
    explicit AndroidAutoMonitor(QObject *parent = nullptr);
    void onConnectionMethodUpdate(Enum::AndroidAutoConnectivityMethod::Value method);
    void onConnectionStateUpdate(Enum::AndroidAutoConnectivityState::Value state);

    Enum::AndroidAutoConnectivityState::Value getState() const;
    Enum::AndroidAutoConnectivityMethod::Value getMethod() const;

    void setState(Enum::AndroidAutoConnectivityState::Value value);
    void setMethod(Enum::AndroidAutoConnectivityMethod::Value value);

  signals:
    void connectivityStateChanged(Enum::AndroidAutoConnectivityState::Value state);
    void connectivityMethodChanged(Enum::AndroidAutoConnectivityMethod::Value method);

  private:
    Enum::AndroidAutoConnectivityMethod::Value m_connectivityMethod;
    Enum::AndroidAutoConnectivityState::Value m_connectivityState;

  };
}
#endif //OPENAUTO_ANDROIDAUTOMONITOR_HPP