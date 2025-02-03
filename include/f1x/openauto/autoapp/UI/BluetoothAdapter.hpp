#include <QtCore/QObject>
#include <QtDBus/QDBusInterface>

namespace f1x::openauto::autoapp::UI {

  class BluetoothAdapter {
  public:

    // Copy Constructor
    BluetoothAdapter(const BluetoothAdapter& other)
        : hardwareAddress(other.hardwareAddress), name(other.name), path(other.path),
          powered(other.powered), discoverable(other.discoverable), discovering(other.discovering) {}

    // Move Constructor
    BluetoothAdapter(BluetoothAdapter&& other) noexcept
        : hardwareAddress(std::move(other.hardwareAddress)), name(std::move(other.name)),
          path(std::move(other.path)), powered(other.powered),
          discoverable(other.discoverable), discovering(other.discovering) {}

    // Copy Assignment Operator
    BluetoothAdapter& operator=(const BluetoothAdapter& other) {
      if (this != &other) {
        hardwareAddress = other.hardwareAddress;
        name = other.name;
        path = other.path;
        powered = other.powered;
        discoverable = other.discoverable;
        discovering = other.discovering;
      }
      return *this;
    }

    // Move Assignment Operator
    BluetoothAdapter& operator=(BluetoothAdapter&& other) noexcept {
      if (this != &other) {
        hardwareAddress = std::move(other.hardwareAddress);
        name = std::move(other.name);
        path = std::move(other.path);
        powered = std::move(other.powered);
        discoverable = std::move(other.discoverable);
        discovering = std::move(other.discovering);
      }
      return *this;
    }

    BluetoothAdapter(const QString& addr, const QString& n, const QString& p, bool pr = false, bool de = false, bool dg = false)
        : hardwareAddress(addr), name(n), path(p), powered(pr), discoverable(de), discovering(dg) {}

    // Default Constructor
    BluetoothAdapter() : BluetoothAdapter("", "", "") {}

    QString hardwareAddress;
    QString name;
    QString path;
    bool powered;
    bool discoverable;
    bool discovering;

    ~BluetoothAdapter();
  };
}