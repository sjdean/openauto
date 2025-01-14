#ifndef OPENAUTO_BLUETOOTHPOPUP_H
#define OPENAUTO_BLUETOOTHPOPUP_H

#include <QtCore/QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::UI {
  enum BluetoothConnectionStatus {
    BC_NOT_CONFIGURED,
    BC_DISCONNECTED,
    BC_CONNECTING,
    BC_CONNECTED
  };

  class BluetoothPopup : public QObject {
     Q_OBJECT

    Q_PROPERTY(QString bluetoothLocalDevice READ getBluetoothLocalDevice WRITE setBluetoothLocalDevice NOTIFY bluetoothLocalDeviceChanged)
    Q_PROPERTY(QString bluetoothConnectedDevice READ getBluetoothConnectedDevice WRITE setBluetoothConnectedDevice NOTIFY bluetoothConnectedDeviceChanged)
    Q_PROPERTY(
        BluetoothConnectionStatus bluetoothStatus READ getBluetoothStatus WRITE setBluetoothStatus NOTIFY bluetoothStatusChanged)
    Q_PROPERTY(int pairedDeviceCount READ getPairedDeviceCount NOTIFY pairedDeviceCountChanged)
    Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
    Q_ENUM(BluetoothConnectionStatus)
  public:
    explicit BluetoothPopup(const QString &hardwareAddress, QObject *parent);

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
    QString getBluetoothLocalDevice();
    void setBluetoothLocalDevice(QString value);

    QString getBluetoothConnectedDevice();
    void setBluetoothConnectedDevice(QString value);

    BluetoothConnectionStatus getBluetoothStatus();

    int getPairedDeviceCount();
    int getConnectedDeviceCount();



    void setBluetoothStatus(BluetoothConnectionStatus value);

    QString m_bluetoothLocalDevice;
    QString m_bluetoothConnectedDevice;
    BluetoothConnectionStatus m_bluetoothStatus;
    int m_pairedDeviceCount;
    int m_connectedDeviceCount;
    void listPairedDevices();

    QString getAdapterPathByAddress(const QString &adapterAddress);

    QString m_adapterPath;
    QDBusInterface m_adapterInterface;

  };
}
#endif //OPENAUTO_BLUETOOTHPOPUP_H