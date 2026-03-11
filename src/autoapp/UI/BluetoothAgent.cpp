#include "f1x/openauto/autoapp/UI/BluetoothAgent.hpp"

#include <QDBusConnection>
#include <QDBusError>
#include <QRandomGenerator>

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBtAgent, "journeyos.bluetoth.agent")


/**
 * @brief BluetoothAgent for QT/BlueZ/Dbus - Handles Pairing
 * @param dbusPath The path to register this object on (e.g., "/org/journeyos/agent")
 */
BluetoothAgent::BluetoothAgent(const QString &dbusPath, QObject *parent) :
    QObject(parent), m_dbusPath(dbusPath)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerObject(m_dbusPath, this)) {
        qWarning(lcBtAgent) << "dbus registration failed path=" << m_dbusPath;
    } else {
        qInfo(lcBtAgent) << "registered path=" << m_dbusPath;
    }
}

BluetoothAgent::~BluetoothAgent() {
    QDBusConnection::systemBus().unregisterObject(m_dbusPath);
}

// --- D-Bus Slots (Called by BlueZ) ---
// TODO: I presume we don't need to explicitly connect to the slots as we have these here registered on the agent?
QString BluetoothAgent::RequestPinCode(const QDBusObjectPath &device) {
    qInfo(lcBtAgent) << "PIN requested device=" << device.path();

    // For legacy pairing, just use "0000" or "1234"
    const QString pin = "0000";
    emit showPinCode(pin);
    return pin;
}

quint32 BluetoothAgent::RequestPasskey(const QDBusObjectPath &device) {
    qInfo(lcBtAgent) << "passkey requested device=" << device.path();
    emit showPinCode(QStringLiteral("000000"));
    return 0;
}

void BluetoothAgent::DisplayPinCode(const QDBusObjectPath &device, const QString &pincode) {
    qInfo(lcBtAgent) << "display PIN device=" << device.path();
    emit showPinCode(pincode);
}

void BluetoothAgent::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered) {
    qInfo(lcBtAgent) << "display passkey=" << passkey << " entered=" << entered << " device=" << device.path();
    emit showConfirmation(QString::number(passkey));
}

void BluetoothAgent::RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey) {
    qInfo(lcBtAgent) << "confirmation requested passkey=" << passkey << " device=" << device.path();

    m_pendingMessage = message;
    emit showConfirmation(QString::number(passkey));
}

void BluetoothAgent::Release() {
    qInfo(lcBtAgent) << "agent released";
    emit pairingComplete();
}

void BluetoothAgent::AuthorizeService(const QDBusObjectPath &device, const QString &uuid) {
    qInfo(lcBtAgent) << "service authorized uuid=" << uuid << " device=" << device.path();
}

void BluetoothAgent::Cancel() {
    qInfo(lcBtAgent) << "pairing cancelled";
    emit pairingComplete();
}

// --- QML-Callable Slots (Called by your UI) ---

void BluetoothAgent::accept() {
    qInfo(lcBtAgent) << "user accepted pairing";
    replyToMessage(true);
    emit pairingComplete();
}

void BluetoothAgent::reject() {
    qInfo(lcBtAgent) << "user rejected pairing";
    replyToMessage(false);
    emit pairingComplete();
}

// --- Private Helper ---

void BluetoothAgent::replyToMessage(bool confirmed) {
    if (m_pendingMessage.service().isEmpty()) {
        qWarning(lcBtAgent) << "no pending message to reply to";
        return;
    }

    if (confirmed) {
        QDBusMessage reply = m_pendingMessage.createReply();
        QDBusConnection::systemBus().send(reply);
    } else {
        QDBusMessage reply = m_pendingMessage.createErrorReply(QDBusError::AccessDenied, "User rejected confirmation");
        QDBusConnection::systemBus().send(reply);
    }

    m_pendingMessage = QDBusMessage();
}
