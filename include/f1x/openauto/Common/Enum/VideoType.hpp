#ifndef OPENAUTO_VIDEOTYPE_HPP
#define OPENAUTO_VIDEOTYPE_HPP

#include <QtCore/QObject>

namespace f1x::openauto::common::Enum {
  class VideoType : public QObject {
  Q_OBJECT

  public:
    enum Value {
      EGL,
      X11
    };

    Q_ENUM(Value);
  };
}

#endif//OPENAUTO_VIDEOTYPE_HPP