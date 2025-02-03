#include <QtCore/QObject>
#include <QtDBus/QDBusInterface>

namespace f1x::openauto::autoapp::UI {

  class BluetoothDevice {
  public:

    // Copy Constructor
    BluetoothDevice(const BluetoothDevice& other)
        : address(other.address), name(other.name), path(other.path),
          paired(other.paired), connected(other.connected) {}

    // Move Constructor
    BluetoothDevice(BluetoothDevice&& other) noexcept
        : address(std::move(other.address)), name(std::move(other.name)),
          path(std::move(other.path)), paired(other.paired),
          connected(other.connected) {}

    // Copy Assignment Operator
    BluetoothDevice& operator=(const BluetoothDevice& other) {
      if (this != &other) {
        address = other.address;
        name = other.name;
        path = other.path;
        paired = other.paired;
        connected = other.connected;
      }
      return *this;
    }

    // Move Assignment Operator
    BluetoothDevice& operator=(BluetoothDevice&& other) noexcept {
      if (this != &other) {
        address = std::move(other.address);
        name = std::move(other.name);
        path = std::move(other.path);
        paired = other.paired;
        connected = other.connected;
      }
      return *this;
    }

    BluetoothDevice(const QString& addr, const QString& n, const QDBusObjectPath& p, bool pr = false, bool c = false)
        : address(addr), name(n), path(p), paired(pr), connected(c) {}

    // Default Constructor
    BluetoothDevice() : BluetoothDevice("", "", QDBusObjectPath("")) {}

    QString address;
    QString name;
    QDBusObjectPath path;
    bool paired;
    bool connected;

    ~BluetoothDevice();
  };
}