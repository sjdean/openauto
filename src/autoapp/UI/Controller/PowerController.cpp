#include "f1x/openauto/autoapp/UI/Controller/PowerController.hpp"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QDebug>

namespace f1x::openauto::autoapp::System {

    PowerController::PowerController(QObject *parent) : QObject(parent) {}

    void PowerController::reboot() {
        qInfo() << "PowerController: Requesting Reboot";
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
        qInfo() << "PowerController: Requesting Power Off";
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