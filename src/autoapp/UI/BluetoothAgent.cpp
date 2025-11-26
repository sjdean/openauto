#include "f1x/openauto/autoapp/UI/BluetoothAgent.hpp"

#include <QDBusConnection>
#include <QDBusError>
#include <QRandomGenerator>

Q_LOGGING_CATEGORY(logAgent, "autoapp.agent")

/**
 * @brief BluetoothAgent for QT/BlueZ/Dbus - Handles Pairing
 * @param dbusPath The path to register this object on (e.g., "/org/journeyos/agent")
 */
BluetoothAgent::BluetoothAgent(const QString &dbusPath, QObject *parent) :
    QObject(parent), m_dbusPath(dbusPath)
{
    // Register this object on the D-Bus system bus
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerObject(m_dbusPath, this)) {
        qWarning(logAgent) << "Failed to register BluetoothAgent on D-Bus path:" << m_dbusPath;
    } else {
        qInfo(logAgent) << "BluetoothAgent registered at" << m_dbusPath;
    }
}

BluetoothAgent::~BluetoothAgent() {
    // Unregister from D-Bus
    QDBusConnection::systemBus().unregisterObject(m_dbusPath);
}

// --- D-Bus Slots (Called by BlueZ) ---
// TODO: I presume we don't need to explicitly connect to the slots as we have these here registered on the agent?
QString BluetoothAgent::RequestPinCode(const QDBusObjectPath &device) {
    qInfo(logAgent) << "Requesting PIN for device" << device.path();

    // For legacy pairing, just use "0000" or "1234"
    // For modern, we'd generate a 6-digit number.
    const QString pin = "0000";

    // 1. Emit the signal to QML
    emit showPinCode(pin);

    // 2. Return the PIN to BlueZ
    return pin;
}

void BluetoothAgent::RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey) {
    qInfo(logAgent) << "Requesting confirmation for passkey:" << passkey << "for device" << device.path();

    // 1. Store the message so we can reply later
    m_pendingMessage = message;

    // 2. Emit the signal to QML
    emit showConfirmation(QString::number(passkey));
}

void BluetoothAgent::Release() {
    qInfo(logAgent) << "Agent released.";
    emit pairingComplete(); // Tell QML to close the popup
}

void BluetoothAgent::AuthorizeService(const QDBusObjectPath &device, const QString &uuid) {
    qInfo(logAgent) << "Authorizing service" << uuid << "for device" << device.path();
    // Auto-accept service authorization (e.g., phone book)
}

void BluetoothAgent::Cancel() {
    qInfo(logAgent) << "Pairing canceled.";
    emit pairingComplete(); // Tell QML to close the popup
}

// --- QML-Callable Slots (Called by your UI) ---

void BluetoothAgent::accept() {
    qInfo(logAgent) << "User accepted pairing.";
    replyToMessage(true);
    emit pairingComplete();
}

void BluetoothAgent::reject() {
    qInfo(logAgent) << "User rejected pairing.";
    replyToMessage(false);
    emit pairingComplete();
}

// --- Private Helper ---

void BluetoothAgent::replyToMessage(bool confirmed) {
    if (!m_pendingMessage.service().isEmpty()) {
        qWarning(logAgent) << "No pending message to reply to!";
        return;
    }

    if (confirmed) {
        // Send a blank "success" reply
        QDBusMessage reply = m_pendingMessage.createReply();
        QDBusConnection::systemBus().send(reply);
    } else {
        // Send an error reply
        QDBusMessage reply = m_pendingMessage.createErrorReply(QDBusError::AccessDenied, "User rejected confirmation");
        QDBusConnection::systemBus().send(reply);
    }

    // Clear the stored message
    m_pendingMessage = QDBusMessage();
}