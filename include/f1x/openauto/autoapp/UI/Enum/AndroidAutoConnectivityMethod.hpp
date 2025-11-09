#ifndef OPENAUTO_ANDROIDAUTOCONNECTIVITYMETHOD_HPP
#define OPENAUTO_ANDROIDAUTOCONNECTIVITYMETHOD_HPP

#include <QObject>

namespace f1x::openauto::autoapp::UI::Enum {

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
}
#endif//OPENAUTO_ANDROIDAUTOCONNECTIVITYMETHOD_HPP