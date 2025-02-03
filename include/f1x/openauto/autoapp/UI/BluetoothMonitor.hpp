#ifndef OPENAUTO_BLUETOOTHMONITOR_HPP
#define OPENAUTO_BLUETOOTHMONITOR_HPP

#include <QtCore/QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include "BluetoothConnectionStatus.hpp"

namespace f1x::openauto::autoapp::UI {

  class BluetoothMonitor : public QObject {
  Q_OBJECT

    Q_PROPERTY(QString bluetoothLocalDevice READ getBluetoothLocalDevice WRITE setBluetoothLocalDevice NOTIFY bluetoothLocalDeviceChanged)
    Q_PROPERTY(QString bluetoothConnectedDevice READ getBluetoothConnectedDevice WRITE setBluetoothConnectedDevice NOTIFY bluetoothConnectedDeviceChanged)
    Q_PROPERTY(
        BluetoothConnectionStatus bluetoothStatus READ getBluetoothStatus NOTIFY bluetoothStatusChanged)
    Q_PROPERTY(int pairedDeviceCount READ getPairedDeviceCount NOTIFY pairedDeviceCountChanged)
    Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)

  public:
    explicit BluetoothMonitor(const QString &hardwareAddress, QObject *parent);

    BluetoothConnectionStatus::Value getBluetoothStatus();

    QString getBluetoothLocalDevice();
    void setBluetoothLocalDevice(QString value);

    QString getBluetoothConnectedDevice();
    void setBluetoothConnectedDevice(QString value);

    int getPairedDeviceCount();
    int getConnectedDeviceCount();

    std::vector<std::pair<std::string, std::string>> getDeviceList();

  signals:

    void bluetoothLocalDeviceChanged();
    void bluetoothConnectedDeviceChanged();
    void bluetoothStatusChanged();
    void pairedDeviceCountChanged();
    void connectedDeviceCountChanged();


  private slots:

    void onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties);
    void onDeviceConnected(const QDBusObjectPath &path);
    void onDeviceDisconnected(const QDBusObjectPath &path);

  private:
    QString getAdapterPathByAddress(const QString &adapterAddress);
    void initialiseMonitor();

    QString m_bluetoothLocalDevice;
    QString m_bluetoothConnectedDevice;
    BluetoothConnectionStatus::Value m_bluetoothStatus;
    int m_pairedDeviceCount;
    int m_connectedDeviceCount;

    QString m_adapterPath;
    QDBusInterface m_adapterInterface;

    std::vector<std::pair<std::string, std::string>> m_devices;

  };
}
#endif //OPENAUTO_BLUETOOTHMONITOR_HPP