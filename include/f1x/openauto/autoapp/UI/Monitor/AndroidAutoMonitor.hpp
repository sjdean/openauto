#ifndef OPENAUTO_ANDROIDAUTOMONITOR_HPP
#define OPENAUTO_ANDROIDAUTOMONITOR_HPP

#include <QObject>
#include <QtQmlIntegration>
#include <f1x/openauto/autoapp/UI/Monitor/IAndroidAutoMonitor.hpp>

#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

  class AndroidAutoMonitor : public QObject, IAndroidAutoMonitor {
    Q_OBJECT
    Q_PROPERTY(common::Enum::AndroidAutoConnectivityState::Value state READ getState WRITE setState NOTIFY connectivityStateChanged)
    Q_PROPERTY(common::Enum::AndroidAutoConnectivityMethod::Value method READ getMethod WRITE setMethod NOTIFY connectivityMethodChanged)
  public:
    explicit AndroidAutoMonitor(QObject *parent = nullptr);
    void onConnectionMethodUpdate(common::Enum::AndroidAutoConnectivityMethod::Value method);
    void onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::Value state);

    common::Enum::AndroidAutoConnectivityState::Value getState() const;

    common::Enum::AndroidAutoConnectivityMethod::Value getMethod() const;

    void setState(common::Enum::AndroidAutoConnectivityState::Value value);
    void setMethod(common::Enum::AndroidAutoConnectivityMethod::Value value);

  signals:
    void connectivityStateChanged(common::Enum::AndroidAutoConnectivityState::Value state);
    void connectivityMethodChanged(common::Enum::AndroidAutoConnectivityMethod::Value method);

  private:
    common::Enum::AndroidAutoConnectivityMethod::Value m_connectivityMethod;
    common::Enum::AndroidAutoConnectivityState::Value m_connectivityState;

  };
}
#endif //OPENAUTO_ANDROIDAUTOMONITOR_HPP