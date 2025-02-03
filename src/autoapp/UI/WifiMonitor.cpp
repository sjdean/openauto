#include <f1x/openauto/autoapp/UI/WifiMonitor.hpp>

namespace f1x::openauto::autoapp::UI {

  /**
   * Monitor signals to bounce up to interface.
   * @param parent
   */
  WifiMonitor::WifiMonitor(QObject *parent) : QObject(parent) {
    if (!QDBusConnection::systemBus().isConnected()) {
      qWarning() << "Cannot connect to the D-Bus session bus.";
      return;
    }

    bool isHotspot = true;

    if (!isHotspot) {

      // wpa_supplicant uses the 'fi.w1.wpa_supplicant1' service when configured for D-Bus
      QDBusConnection::systemBus().connect("fi.w1.wpa_supplicant1", "/fi/w1/wpa_supplicant1",
                                           "fi.w1.wpa_supplicant1.Interface", "StateChanged", this,
                                           SLOT(onStateChanged(int, QString)));
    } else {

      // Assuming hostapd uses 'fi.w1.hostapd1' for D-Bus (this might vary)
      QDBusConnection::systemBus().connect("fi.w1.hostapd1", "/fi/w1/hostapd1", "fi.w1.hostapd1", "AP-STA-CONNECTED",
                                           this, SLOT(onStaConnected(QString, QString)));
      QDBusConnection::systemBus().connect("fi.w1.hostapd1", "/fi/w1/hostapd1", "fi.w1.hostapd1", "AP-STA-DISCONNECTED",
                                           this, SLOT(onStaDisconnected(QString, QString)));
    }
  }

  /**
   * Update notification when device is connected.
   * @param address Connected Device Address
   * @param interface
   */
  void WifiMonitor::onStaConnected(const QString &address, const QString &interface) {
    qDebug() << "Station" << address << "connected on interface" << interface;
    updateIcon(interface, "green");
  }

  /**
   * Update notification when device is disconnected
   * @param address Connected Device Address
   * @param interface
   */
  void WifiMonitor::onStaDisconnected(const QString &address, const QString &interface) {
    qDebug() << "Station" << address << "disconnected from interface" << interface;
    updateIcon(interface, "red");
  }

  void WifiMonitor::onStateChanged(int newState, const QString &interface) {
    // wpa_supplicant state codes:
    // 0 = Disconnected, 1 = Inactive, 2 = Scanning, 3 = Associating, 4 = Associated, 5 = 4-way handshake, 6 = Group handshake, 7 = Completed
    qDebug() << "WiFi state changed on interface" << interface << "to state:" << newState;
    switch (newState) {
      case 0: updateIcon(interface, "red"); break; // Disconnected
      case 7: updateIcon(interface, "green"); break; // Fully connected
      default: updateIcon(interface, "yellow"); break; // In connecting process
    }
  }

  void WifiMonitor::updateIcon(const QString &interface, const QString &color) {
    qDebug() << "Updating icon for WiFi interface" << interface << "to color" << color;
    // Implement your UI update logic here
  }
};

/*


Note: You'll need to configure wpa_supplicant to use D-Bus. This typically involves adding dbus_ctrl_interface=DBUS_CTRL_INTERFACE to your wpa_supplicant.conf.

Using NetworkManager via D-Bus:
NetworkManager: This provides a higher-level API for network management, including WiFi, and is often used in desktop environments.
D-Bus Interface: NetworkManager has a D-Bus API where you can listen for StateChanged signals from the org.freedesktop.NetworkManager service.


#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

class WiFiMonitorNM : public QObject {
Q_OBJECT

public:
  WiFiMonitorNM(QObject *parent = nullptr) : QObject(parent) {
    if (!QDBusConnection::systemBus().isConnected()) {
      qWarning() << "Cannot connect to the D-Bus session bus.";
      return;
    }

    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "StateChanged", this, SLOT(onNetworkStateChanged(uint)));
  }

private slots:
  void onNetworkStateChanged(uint state) {
    // Some common states:
    // 70 = Disconnected, 30 = Connecting, 100 = Connected
    qDebug() << "Network state changed to:" << state;
    switch (state) {
      case 70: updateIcon("wifi", "red"); break; // Disconnected
      case 100: updateIcon("wifi", "green"); break; // Connected
      default: updateIcon("wifi", "yellow"); break; // Connecting or other states
    }
  }

  void updateIcon(const QString &interface, const QString &color) {
    qDebug() << "Updating icon for WiFi" << interface << "to color" << color;
    // Implement your UI update logic here
  }
};

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  WiFiMonitorNM monitor;

  return a.exec();
}

#include "main.moc"


Considerations:
Permissions: Make sure your application has the necessary permissions to interact with these system services.
Configuration: Depending on your setup, you might need to adjust configurations for wpa_supplicant or NetworkManager to expose or enable the D-Bus interface.
Stability and Updates: The behavior of these services can change with different versions of software on Debian, so testing is crucial.
Fallback: Consider implementing a fallback method (like polling) in case the D-Bus interface isn't available or fails.

This setup will allow you to monitor WiFi state changes and update your UI accordingly. Remember, the exact signals and methods might vary slightly depending on the version of the software you're using on Debian.
 */