#include <f1x/openauto/autoapp/UI/AndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::UI {
  AndroidAutoMonitor::AndroidAutoMonitor(QObject *parent) :
      QObject(parent) {
  }

  void AndroidAutoMonitor::onConnectionMethodUpdate(AndroidAutoConnectivityMethod::Value method) {
    if (m_connectivityMethod != method) {
      m_connectivityMethod = method;
      emit connectivityMethodChanged(method);
    }
  }

  void AndroidAutoMonitor::onConnectionStateUpdate(AndroidAutoConnectivityState::Value state) {
    if (m_connectivityState != state) {
      m_connectivityState = state;
      emit connectivityStateChanged(state);
    }
  }

  AndroidAutoConnectivityState::Value AndroidAutoMonitor::getState() {
    return m_connectivityState;
  }

  AndroidAutoConnectivityMethod::Value AndroidAutoMonitor::getMethod() {
    return m_connectivityMethod;
  }

  void AndroidAutoMonitor::setState(AndroidAutoConnectivityState::Value value) {
    if (m_connectivityState != value) {
      m_connectivityState = value;
      emit connectivityStateChanged(value);
    }
  }

  void AndroidAutoMonitor::setMethod(AndroidAutoConnectivityMethod::Value value) {
    if (m_connectivityMethod != value) {
      m_connectivityMethod = value;
      emit connectivityMethodChanged(value);
    }
  }
}