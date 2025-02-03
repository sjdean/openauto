#ifndef OPENAUTO_BLUETOOTHPOPUP_HPP
#define OPENAUTO_BLUETOOTHPOPUP_HPP

#include <QtCore/QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include "BluetoothConnectionStatus.hpp"

namespace f1x::openauto::autoapp::UI {

  struct BluetoothDevice {
    QString address;
    QString name;
    QDBusObjectPath path;
  };
  Q_DECLARE_METATYPE(BluetoothDevice)

  class BluetoothPopup : public QObject {
     Q_OBJECT

    Q_PROPERTY(QString bluetoothLocalDevice READ getBluetoothLocalDevice WRITE setBluetoothLocalDevice NOTIFY bluetoothLocalDeviceChanged)
    Q_PROPERTY(BluetoothDevice bluetoothConnectedDevice READ getBluetoothConnectedDevice WRITE setBluetoothConnectedDevice NOTIFY bluetoothConnectedDeviceChanged)
    Q_PROPERTY(
        BluetoothConnectionStatus bluetoothStatus READ getBluetoothStatus NOTIFY bluetoothStatusChanged)
    Q_PROPERTY(int pairedDeviceCount READ getPairedDeviceCount NOTIFY pairedDeviceCountChanged)
    Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
    Q_PROPERTY(bool isScanning READ getIsScanning NOTIFY isScanningChanged)
    Q_PROPERTY(QList<BluetoothDevice> discoveredDevices READ getDiscoveredDevices NOTIFY discoveredDevicesChanged)

  public:
    explicit BluetoothPopup(const QString &hardwareAddress, QObject *parent);
    bool doConnectToPairedDevice(BluetoothDevice device);
    bool doRemovePair(const BluetoothDevice& device);
    bool doRemoveAllPairs();
    QList<BluetoothDevice> getDiscoveredDevices() const;
    BluetoothDevice* getBluetoothConnectedDevice();
    void setBluetoothConnectedDevice(BluetoothDevice* value);

  signals:

    void bluetoothLocalDeviceChanged();
    void bluetoothConnectedDeviceChanged();
    void bluetoothStatusChanged();
    void pairedDeviceCountChanged();
    void connectedDeviceCountChanged();
    void discoveredDevicesChanged();


  private slots:

    void onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties);
    void onDeviceConnected(const QDBusObjectPath &path);
    void onDeviceDisconnected(const QDBusObjectPath &path);

  private:
    QString getBluetoothLocalDevice();
    void setBluetoothLocalDevice(QString value);



    BluetoothConnectionStatus::Value getBluetoothStatus();

    int getPairedDeviceCount();
    int getConnectedDeviceCount();

    QString m_bluetoothLocalDevice;
    BluetoothDevice* m_bluetoothConnectedDevice;
    BluetoothConnectionStatus::Value m_bluetoothStatus;
    int m_pairedDeviceCount;
    int m_connectedDeviceCount;
    void listPairedDevices();

    QString getAdapterPathByAddress(const QString &adapterAddress);

    QString m_adapterPath;
    QDBusInterface m_adapterInterface;

    void disconnectCurrentdevice(const QDBusInterface &adapter, const QString devicePath);

    bool connectToDevice(const QDBusInterface &adapter);

    bool connectToDevice(const QDBusInterface &adapter, const QString devicePath, const QString deviceAddress);

    bool doScanDevicesForPairing();

    bool connectToDevice(const BluetoothDevice& device);
    bool disconnectCurrentDevice();

    QList<BluetoothDevice> m_foundDevices;

    BluetoothDevice* findDeviceByPath(const QDBusObjectPath &path);

  };
}
#endif //OPENAUTO_BLUETOOTHPOPUP_HPP