#include <f1x/openauto/autoapp/UI/AndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::UI {
  AndroidAutoMonitor::AndroidAutoMonitor(QObject *parent) :
      QObject(parent) {
  }


  void AndroidAutoMonitor::onConnectionMethodUpdate(f1x::openauto::autoapp::UI::AndroidAutoConnectivityMethod method) {
    if (m_connectivityMethod != method) {
      m_connectivityMethod = method;
      emit connectivityMethodChanged(method);
    }
  }

  void AndroidAutoMonitor::onConnectionStateUpdate(AndroidAutoConnectivityState state) {
    if (m_connectivityState != state) {
      m_connectivityState = state;
      emit connectivityStateChanged(state);
    }
  }
}