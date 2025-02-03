#include <QtCore/QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtCore/qtimer.h>
#include <f1x/openauto/autoapp/UI/BluetoothDevice.hpp>
#include <f1x/openauto/autoapp/UI/BluetoothAdapter.hpp>
#include <f1x/openauto/autoapp/UI/BluetoothConnectionStatus.hpp>

namespace f1x::openauto::autoapp::UI {

  class BluetoothHandler : public QObject {
      Q_OBJECT

      Q_PROPERTY(QList<BluetoothAdapter> bluetoothAdapterList READ getBluetoothAdapterList NOTIFY bluetoothAdapterListChanged)
    Q_PROPERTY(QList<BluetoothDevice> unpairedDeviceList READ getUnpairedDeviceList NOTIFY unpairedDeviceListChanged)
    Q_PROPERTY(QList<BluetoothDevice> pairedDeviceList READ getPairedDeviceList NOTIFY pairedDeviceListChanged)
    Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
    Q_PROPERTY(int adapterCount READ getAdapterCount NOTIFY adapterCountChanged)
    Q_PROPERTY(int activeDeviceIndex READ getActiveDeviceIndex NOTIFY activeDeviceIndexChanged)
    Q_PROPERTY(
        BluetoothConnectionStatus::Value bluetoothConnectionStatus READ getBluetoothConnectionStatus NOTIFY bluetoothConnnectionStatusChanged)
  public:
    BluetoothHandler(QString hardwareAddress);
      QList<BluetoothAdapter> getBluetoothAdapterList();


      bool doConnectToPairedDevice(BluetoothDevice device);
      bool doRemovePair(const BluetoothDevice& device);
      bool doRemoveAllPairs();
      bool doScanDevicesForPairing();
      BluetoothConnectionStatus::Value getBluetoothConnectionStatus();
      void setBluetoothConnectionStatus(BluetoothConnectionStatus::Value value);

  signals:
    void bluetoothAdapterListChanged();
      void bluetoothConnnectionStatusChanged();
    void unpairedDeviceListChanged();
      void pairedDeviceListChanged();
      void connectedDeviceCountChanged();
      void adapterCountChanged();
      void activeDeviceIndexChanged();

  private slots:
    void onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties);
    void onDeviceFound(const QDBusObjectPath &path, const QVariantMap &properties);
    void onDeviceConnected(const QDBusObjectPath &path);
    void onDeviceDisconnected(const QDBusObjectPath &path);

  private:
    bool disconnectCurrentDevice();
    bool connectToDevice(const BluetoothDevice& device);

    QDBusInterface m_manager;
    QList<BluetoothAdapter> m_adapters;
    int m_activeAdapterIndex = -1;

    QList<BluetoothDevice> m_devices;
    int m_activeDeviceIndex = -1;

    int m_connectedDeviceCount;
    BluetoothConnectionStatus::Value m_bluetoothConnectionStatus;

  };
}