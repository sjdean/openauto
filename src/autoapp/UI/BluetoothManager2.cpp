/*#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

class BluetoothMonitor : public QObject {
Q_OBJECT

public:
  BluetoothMonitor(QObject *parent = nullptr) : QObject(parent) {
    if (!QDBusConnection::systemBus().isConnected()) {
      qWarning() << "Cannot connect to the D-Bus session bus.";
      return;
    }

    // Connect to BlueZ signals for device changes
    QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.Adapter1", "DeviceFound", this, SLOT(onDeviceFound(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.Adapter1", "DeviceConnected", this, SLOT(onDeviceConnected(QDBusObjectPath)));
    QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.Adapter1", "DeviceDisconnected", this, SLOT(onDeviceDisconnected(QDBusObjectPath)));

    // Setup for pairing agent (simplified)
    setupPairingAgent();
  }

private slots:
  // Slot for when a new device is found during discovery
  void onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties) {
    qDebug() << "Device found:" << properties.value("Address").toString();
    // Update icon to yellow for connection request
    updateIcon(properties.value("Address").toString(), "yellow");
  }

  // Slot for when a device connects
  void onDeviceConnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();
      qDebug() << "Device connected:" << properties.value("Address").toString();
      // Update icon to green for connected device
      updateIcon(properties.value("Address").toString(), "green");
    }
  }

  // Slot for when a device disconnects
  void onDeviceDisconnected(const QDBusObjectPath &path) {
    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus(), this);
    QDBusReply<QVariantMap> props = device.call("GetProperties");
    if (props.isValid()) {
      QVariantMap properties = props.value();
      qDebug() << "Device disconnected:" << properties.value("Address").toString();
      // Update icon back to red for disconnected device
      updateIcon(properties.value("Address").toString(), "red");
    }
  }

  // Placeholder for icon update, you might want to integrate with a GUI or another system
  void updateIcon(const QString &deviceAddress, const QString &color) {
    qDebug() << "Updating icon for device" << deviceAddress << "to color" << color;
    // Here you would implement the actual icon update, perhaps using QPixmap or similar
  }

  // Simplified setup of a pairing agent
  void setupPairingAgent() {
    QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "Release", this, SLOT(onRelease()));
    QDBusConnection::systemBus().connect("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RequestDefaultAgent", this, SLOT(onRequestDefaultAgent(QDBusObjectPath)));

    QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus(), this);
    agentManager.call("RegisterAgent", QDBusObjectPath("/example/agent"), QString("DisplayOnly")); // or "KeyboardDisplay"
    agentManager.call("RequestDefaultAgent", QDBusObjectPath("/example/agent"));
  }

  void onRelease() {
    qDebug() << "Agent release signal received";
  }

  void onRequestDefaultAgent(const QDBusObjectPath &path) {
    qDebug() << "Default agent request for" << path.path();
  }

};

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  BluetoothMonitor monitor;

  return a.exec();
}

#include "main.moc"*/