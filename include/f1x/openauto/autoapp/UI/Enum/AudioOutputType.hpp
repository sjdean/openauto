#ifndef OPENAUTO_AUDIOOUTPUTTYPE_HPP
#define OPENAUTO_AUDIOOUTPUTTYPE_HPP

#include <QtCore/QObject>

namespace f1x::openauto::autoapp::UI::Enum  {
    class AudioOutputType : public QObject {
        Q_OBJECT

        public:
        enum Value {
            RTAUDIO = 1,
            QT = 2
          };

        Q_ENUM(Value);
    };
}

#endif//OPENAUTO_AUDIOOUTPUTTYPE_HPP