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
        // Use systemd-logind directly — no custom helper service required.
        // interactive=false: do not prompt via polkit; journeyos has the policy grant.
        QDBusMessage msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.login1"),
                QStringLiteral("/org/freedesktop/login1"),
                QStringLiteral("org.freedesktop.login1.Manager"),
                QStringLiteral("Reboot")
            );
        msg.setArguments({false});
        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
#endif
    }

    void PowerController::powerOff() {
        qInfo(lcPower) << "power off requested";
#ifdef Q_OS_LINUX
        QDBusMessage msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.freedesktop.login1"),
                QStringLiteral("/org/freedesktop/login1"),
                QStringLiteral("org.freedesktop.login1.Manager"),
                QStringLiteral("PowerOff")
            );
        msg.setArguments({false});
        QDBusConnection::systemBus().call(msg, QDBus::NoBlock);
#endif
    }

}