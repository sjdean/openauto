#pragma once
#include <QObject>

namespace f1x::openauto::autoapp::System {

    class PowerController : public QObject {
        Q_OBJECT
    public:
        explicit PowerController(QObject *parent = nullptr);

        Q_INVOKABLE void reboot();
        Q_INVOKABLE void powerOff();
    };

}