#ifndef OPENAUTO_BLUETOOTHCONNECTIONSTATUS_HPP
#define OPENAUTO_BLUETOOTHCONNECTIONSTATUS_HPP

#include <QtCore/QObject>

namespace f1x::openauto::autoapp::UI::Enum  {
  class BluetoothConnectionStatus : public QObject {
  Q_OBJECT

  public:
    enum Value {
      BC_NOT_CONFIGURED,
      BC_DISCONNECTED,
      BC_CONNECTING,
      BC_CONNECTED
    };

    Q_ENUM(Value)
  };
}
#endif//OPENAUTO_BLUETOOTHCONNECTIONSTATUS_HPP