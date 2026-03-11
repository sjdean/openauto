#include "f1x/openauto/autoapp/UI/Controller/PowerController.hpp"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcPower, "journeyos.power")

namespace f1x::openauto::autoapp::System {

    PowerController::PowerController(QObject *parent) : QObject(parent) {}

    void PowerController::reboot() {
        qInfo(lcPower) << "reboot requested";
#ifdef Q_OS_LINUX
        QDBusMessage msg = QDBusMessage::createMethodCall(
                "uk.co.cubeone.journeyos.helper",
                "/uk/co/cubeone/journeyos/helper",
                "uk.co.cubeone.journeyos.helper",
                "reboot"
            );
        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
#endif
    }

    void PowerController::powerOff() {
        qInfo(lcPower) << "power off requested";
#ifdef Q_OS_LINUX
        QDBusMessage msg = QDBusMessage::createMethodCall(
                "uk.co.cubeone.journeyos.helper",
                "/uk/co/cubeone/journeyos/helper",
                "uk.co.cubeone.journeyos.helper",
                "powerOff"
            );
        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
#endif
    }

}