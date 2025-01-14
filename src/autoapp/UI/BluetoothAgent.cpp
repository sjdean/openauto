#include <f1x/openauto/autoapp/UI/BluetoothAgent.hpp>

BluetoothAgent::BluetoothAgent(QObject *parent) :
    QObject(parent) {
}

// Method called when a PIN code is needed during pairing
QString BluetoothAgent::RequestPinCode(const QDBusObjectPath &device) {
  qDebug() << "Requesting PIN for device" << device.path();
  // Here, you would typically ask the user for a PIN or generate one
  return "1234"; // Example PIN
}

// Method called when numeric comparison is needed
void BluetoothAgent::RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey) {
  qDebug() << "Confirm passkey" << passkey << "for device" << device.path();
  confirmPasskey(message, device, passkey, true);
}

// Method called when the agent should be released
void BluetoothAgent::Release() {
  qDebug() << "Agent released.";
}

// Helper method to confirm passkey if needed
void BluetoothAgent::confirmPasskey(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey, bool confirmed) {

  if (confirmed) {
    QDBusMessage reply = message.createReply();
    reply << QVariant();
    QDBusConnection::systemBus().send(reply);
  } else {
    QDBusMessage reply = message.createErrorReply(QDBusError::Failed, "User rejected confirmation");
    QDBusConnection::systemBus().send(reply);
  }
}
