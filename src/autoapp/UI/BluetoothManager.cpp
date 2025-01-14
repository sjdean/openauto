/*
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QList>

// Helper function to convert QDBusArgument to QVariantMap
static QVariantMap dbusArgumentToVariantMap(const QDBusArgument &arg)
{
  QVariantMap map;
  arg.beginMap();
  while (!arg.atEnd()) {
    QString key;
    QVariant value;
    arg.beginMapEntry();
    arg >> key >> value;
    arg.endMapEntry();
    map.insert(key, value);
  }
  arg.endMap();
  return map;
}
class BluetoothManager : public QObject {
  Q_OBJECT

public:
  BluetoothManager(QObject *parent = nullptr) : QObject(parent) {
    connectToBluez();
  }

  void connectToBluez() {
    if (!QDBusConnection::systemBus().isConnected()) {
      qWarning() << "Cannot connect to the D-Bus session bus.\n"
                 << "To start it, run:\n"
                 << "\teval `dbus-launch --sh-syntax`\n";
      return;
    }
    m_adapterInterface = new QDBusInterface("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", QDBusConnection::systemBus(), this);
    m_managerInterface = new QDBusInterface("org.bluez", "/", "org.bluez.Manager", QDBusConnection::systemBus(), this);

    if (!m_adapterInterface->isValid()) {
      qWarning() << "Adapter interface not valid.";
    }
    if (!m_managerInterface->isValid()) {
      qWarning() << "Manager interface not valid.";
    }
  }

  // 1) List all Paired Devices
  void listPairedDevices() {
    auto reply = m_adapterInterface->call("GetProperties");
    if (reply.type() == QDBusMessage::ReplyMessage) {
      QVariantMap properties = dbusArgumentToVariantMap(reply.arguments().at(0).value<QDBusArgument>());
      QVariant devices = properties.value("Devices");
      if (devices.isValid()) {
        QList<QDBusObjectPath> devicePaths = devices.value<QList<QDBusObjectPath>>();
        for (const QDBusObjectPath &path : devicePaths) {
          QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus());
          QDBusReply<QVariantMap> deviceProperties = device.call("GetProperties");
          if (deviceProperties.isValid()) {
            QVariantMap prop = deviceProperties.value();
            if (prop.value("Paired").toBool()) {
              qDebug() << "Paired device:" << prop.value("Alias").toString() << "with address:" << prop.value("Address").toString();
            }
          }
        }
      }
    }
  }

  // 2) Show Pairing Statuses
  void showPairingStatuses() {
    auto reply = m_adapterInterface->call("GetProperties");
    if (reply.type() == QDBusMessage::ReplyMessage) {
      QVariantMap properties = dbusArgumentToVariantMap(reply.arguments().at(0).value<QDBusArgument>());
      QVariant devices = properties.value("Devices");
      if (devices.isValid()) {
        QList<QDBusObjectPath> devicePaths = devices.value<QList<QDBusObjectPath>>();
        for (const QDBusObjectPath &path : devicePaths) {
          QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus());
          QDBusReply<QVariantMap> deviceProperties = device.call("GetProperties");
          if (deviceProperties.isValid()) {
            QVariantMap prop = deviceProperties.value();
            qDebug() << "Device:" << prop.value("Alias").toString()
                     << ", Paired:" << prop.value("Paired").toBool()
                     << ", Connected:" << prop.value("Connected").toBool();
          }
        }
      }
    }
  }

  // 3) Respond to Pairing
  void setupPairingAgent() {
    // This is a simplified version, actual implementation requires more logic for passkey handling
    QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus(), this);
    QDBusInterface agent("org.example", "/example/agent", "org.bluez.Agent1", QDBusConnection::systemBus(), this);

    // Register the agent
    agentManager.call("RegisterAgent", QDBusObjectPath("/example/agent"), QString("DisplayOnly")); // or "KeyboardDisplay"
    agentManager.call("RequestDefaultAgent", QDBusObjectPath("/example/agent"));

    // For full implementation, you'd define slots for methods like RequestPinCode, DisplayPasskey, etc.
  }

  // 4) Initiate Pairing from C++ and list devices to pair with
  void initiatePairing(const QString &deviceAddress) {
    QDBusInterface adapter("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> devicesReply = adapter.call("GetDevices");
    if (devicesReply.isValid()) {
      for (const QDBusObjectPath &path : devicesReply.value()) {
        QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus());
        QDBusReply<QVariantMap> deviceProperties = device.call("GetProperties");
        if (deviceProperties.isValid()) {
          QVariantMap prop = deviceProperties.value();
          if (prop.value("Address").toString() == deviceAddress) {
            device.call("Pair");
            break;
          }
        }
      }
    }
  }

  // 5) Auto Connect to Previous paired devices on program start
  void autoConnectToPairedDevices() {
    listPairedDevices(); // First, list paired devices to get their paths
    // Here you would loop through those devices and try to connect each one:
    // Note: This is pseudo-code since actual connection depends on device properties and profiles
    for (auto path : /* paths from listPairedDevices */ //) {
  /*    QDBusInterface device("org.bluez", path.path(), "org.bluez.Device1", QDBusConnection::systemBus());
      device.call("Connect"); // This might require specifying a profile or could fail if the device doesn't support immediate connect after pairing
    }
  }
};

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  BluetoothManager manager;

  manager.listPairedDevices();
  manager.showPairingStatuses();
  manager.setupPairingAgent();
  // Example: manager.initiatePairing("XX:XX:XX:XX:XX:XX");
  manager.autoConnectToPairedDevices();

  return a.exec();
}

#include "main.moc"

   */