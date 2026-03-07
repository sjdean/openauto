#pragma once
#include <QtCore/QObject>

#ifdef Q_OS_LINUX
#include <QtDBus/QDBusObjectPath>
#endif

namespace f1x::openauto::autoapp::UI::Model {

  class BluetoothDevice {

  public:

    // Copy Constructor
    BluetoothDevice(const BluetoothDevice& other)
        : address(other.address), name(other.name),
#ifdef Q_OS_LINUX
          path(other.path),
#endif
          paired(other.paired), connected(other.connected) {}

    // Move Constructor
    BluetoothDevice(BluetoothDevice&& other) noexcept
        : address(std::move(other.address)), name(std::move(other.name)),
#ifdef Q_OS_LINUX
          path(std::move(other.path)),
#endif
          paired(other.paired),
          connected(other.connected) {}

    // Copy Assignment Operator
    BluetoothDevice& operator=(const BluetoothDevice& other) {
      if (this != &other) {
        address = other.address;
        name = other.name;
#ifdef Q_OS_LINUX
        path = other.path;
#endif
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
#ifdef Q_OS_LINUX
        path = std::move(other.path);
#endif
        paired = other.paired;
        connected = other.connected;
      }
      return *this;
    }

#ifdef Q_OS_LINUX
    BluetoothDevice(const QString& addr, const QString& n, const QDBusObjectPath& p, bool pr = false, bool c = false)
        : address(addr), name(n), path(p), paired(pr), connected(c) {}
#else
    BluetoothDevice(const QString& addr, const QString& n, const QString& p = {}, bool pr = false, bool c = false)
        : address(addr), name(n), path(p), paired(pr), connected(c) {}
#endif

    // Default Constructor
    BluetoothDevice() : BluetoothDevice("", "", {}) {}

    QString address;
    QString name;
#ifdef Q_OS_LINUX
    QDBusObjectPath path;
#else
    QString path;
#endif
    bool paired = false;
    bool connected = false;

    ~BluetoothDevice() = default;
  };
}
