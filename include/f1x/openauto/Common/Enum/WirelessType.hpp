#ifndef OPENAUTO_WIRELESSTYPE_HPP
#define OPENAUTO_WIRELESSTYPE_HPP

#include <QtCore/QObject>

namespace f1x::openauto::common::Enum {
  class WirelessType : public QObject {
    Q_OBJECT

  public:
    enum Value {
      WIRELESS_HOTSPOT,
      WIRELESS_CLIENT
    };

    Q_ENUM(Value);
  };
}
#endif//OPENAUTO_WIRELESSTYPE_HPP