#ifndef OPENAUTO_ANDROIDAUTOCONNECTIVITYSTATE_HPP
#define OPENAUTO_ANDROIDAUTOCONNECTIVITYSTATE_HPP

#include <QObject>

namespace f1x::openauto::common::Enum {

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
}
#endif//OPENAUTO_ANDROIDAUTOCONNECTIVITYSTATE_HPP