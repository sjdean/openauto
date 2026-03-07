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
    // Register this object on the D-Bus system bus
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerObject(m_dbusPath, this)) {
        qWarning(lcBtAgent) << "Failed to register BluetoothAgent on D-Bus path:" << m_dbusPath;
    } else {
        qInfo(lcBtAgent) << "BluetoothAgent registered at" << m_dbusPath;
    }
}

BluetoothAgent::~BluetoothAgent() {
    // Unregister from D-Bus
    QDBusConnection::systemBus().unregisterObject(m_dbusPath);
}

// --- D-Bus Slots (Called by BlueZ) ---
// TODO: I presume we don't need to explicitly connect to the slots as we have these here registered on the agent?
QString BluetoothAgent::RequestPinCode(const QDBusObjectPath &device) {
    qInfo(lcBtAgent) << "Requesting PIN for device" << device.path();

    // For legacy pairing, just use "0000" or "1234"
    // For modern, we'd generate a 6-digit number.
    const QString pin = "0000";

    // 1. Emit the signal to QML
    emit showPinCode(pin);

    // 2. Return the PIN to BlueZ
    return pin;
}

quint32 BluetoothAgent::RequestPasskey(const QDBusObjectPath &device) {
    qInfo(lcBtAgent) << "Requesting passkey for device" << device.path();
    // Return 0 as a default passkey and display it to the user
    emit showPinCode(QStringLiteral("000000"));
    return 0;
}

void BluetoothAgent::DisplayPinCode(const QDBusObjectPath &device, const QString &pincode) {
    qInfo(lcBtAgent) << "Display PIN code" << pincode << "for device" << device.path();
    emit showPinCode(pincode);
}

void BluetoothAgent::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint16 entered) {
    qInfo(lcBtAgent) << "Display passkey" << passkey << "(entered:" << entered << ") for device" << device.path();
    emit showConfirmation(QString::number(passkey));
}

void BluetoothAgent::RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey) {
    qInfo(lcBtAgent) << "Requesting confirmation for passkey:" << passkey << "for device" << device.path();

    // 1. Store the message so we can reply later
    m_pendingMessage = message;

    // 2. Emit the signal to QML
    emit showConfirmation(QString::number(passkey));
}

void BluetoothAgent::Release() {
    qInfo(lcBtAgent) << "Agent released.";
    emit pairingComplete(); // Tell QML to close the popup
}

void BluetoothAgent::AuthorizeService(const QDBusObjectPath &device, const QString &uuid) {
    qInfo(lcBtAgent) << "Authorizing service" << uuid << "for device" << device.path();
    // Auto-accept service authorization (e.g., phone book)
}

void BluetoothAgent::Cancel() {
    qInfo(lcBtAgent) << "Pairing canceled.";
    emit pairingComplete(); // Tell QML to close the popup
}

// --- QML-Callable Slots (Called by your UI) ---

void BluetoothAgent::accept() {
    qInfo(lcBtAgent) << "User accepted pairing.";
    replyToMessage(true);
    emit pairingComplete();
}

void BluetoothAgent::reject() {
    qInfo(lcBtAgent) << "User rejected pairing.";
    replyToMessage(false);
    emit pairingComplete();
}

// --- Private Helper ---

void BluetoothAgent::replyToMessage(bool confirmed) {
    if (m_pendingMessage.service().isEmpty()) {
        qWarning(lcBtAgent) << "No pending message to reply to!";
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