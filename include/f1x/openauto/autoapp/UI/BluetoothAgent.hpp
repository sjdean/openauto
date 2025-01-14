#ifndef OPENAUTO_BLUETOOTHAGENT_HPP
#define OPENAUTO_BLUETOOTHAGENT_HPP

#include <QtCore/QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDbus/QDBusObjectPath>
#include <QDebug>

class BluetoothAgent : public QObject {
  Q_OBJECT

public:
  explicit BluetoothAgent(QObject *parent = nullptr);

public slots:
  QString RequestPinCode(const QDBusObjectPath &device);
  void RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey);
  void Release();

private:
  void confirmPasskey(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey, bool confirmed);
};
#endif //OPENAUTO_BLUETOOTHAGENT_HPP