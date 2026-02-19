#pragma once
#include <QtCore/QObject>
#include <QTimer>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "f1x/openauto/autoapp/UI/Model/BluetoothDevice.hpp"
#include "f1x/openauto/autoapp/UI/Model/BluetoothAdapter.hpp"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

#if defined(__LINUX__)
#include "f1x/openauto/autoapp/UI/BluetoothAgent.hpp"
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#endif

#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#include "f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp"


namespace f1x::openauto::autoapp::UI::Monitor {
    class BluetoothHandler : public QObject {
        Q_OBJECT
        // Properties
        Q_PROPERTY(QVariantList bluetoothAdapterList READ getBluetoothAdapterList NOTIFY bluetoothAdapterListChanged)
        Q_PROPERTY(QVariantList unpairedDeviceList READ getUnpairedDeviceList NOTIFY unpairedDeviceListChanged)
        Q_PROPERTY(QVariantList pairedDeviceList READ getPairedDeviceList NOTIFY pairedDeviceListChanged)
        Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
        Q_PROPERTY(int adapterCount READ getAdapterCount NOTIFY adapterCountChanged)
        Q_PROPERTY(int activeDeviceIndex READ getActiveDeviceIndex NOTIFY activeDeviceIndexChanged)
        Q_PROPERTY(
            f1x::openauto::common::Enum::BluetoothConnectionStatus::Value bluetoothConnectionStatus READ
            getBluetoothConnectionStatus NOTIFY bluetoothConnnectionStatusChanged)
        Q_PROPERTY(QString statusText READ getStatusText NOTIFY bluetoothConnnectionStatusChanged)
        Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
        // Agent property only valid on Linux, or return nullptr on Mac
        Q_PROPERTY(QObject* agent READ getAgent CONSTANT)

    public:
        explicit BluetoothHandler(configuration::IConfiguration::Pointer configuration,
                                  QObject *parent = nullptr);

        // Public Q_INVOKABLEs (Called by UI)
        Q_INVOKABLE QString getAdapterAddress() const;

        Q_INVOKABLE void setActiveAdapter(const QString &address);

        Q_INVOKABLE void startScan();

        Q_INVOKABLE void pair(const QString &address);

        Q_INVOKABLE bool doConnectToPairedDevice(const QString &address);
        Q_INVOKABLE void doDisconnect(const QString &address);

        Q_INVOKABLE bool doRemovePair(const QString &address);

        Q_INVOKABLE bool doRemoveAllPairs();

        QVariantList getBluetoothAdapterList();

        int getConnectedDeviceCount() const;

        int getAdapterCount() const;

        int getActiveDeviceIndex() const;

        common::Enum::BluetoothConnectionStatus::Value getBluetoothConnectionStatus() const;

        void setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::Value value);

        QString getStatusText() const;

        bool isScanning() const;

        Q_INVOKABLE void ignoreDevice(const QString &address);

        QVariantList getPairedDeviceList();

        QVariantList getUnpairedDeviceList();

        QObject *getAgent() const {
#ifdef Q_OS_LINUX
            return m_agent;
#else
            return nullptr;
#endif
        }

    signals:
        void bluetoothAdapterListChanged();

        void unpairedDeviceListChanged();

        void pairedDeviceListChanged();

        void bluetoothConnnectionStatusChanged();

        void connectedDeviceCountChanged();

        void adapterCountChanged();

        void activeDeviceIndexChanged();

        void isScanningChanged();

        void pairingPinConfirmation(const QString &pin, const QString &deviceAddress);

    private slots:
        void onDeviceDiscovered(const QBluetoothDeviceInfo &info);

        void onScanFinished();

        void onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

        // Linux Agent Slot
#ifdef Q_OS_LINUX
        void onAgentPinRequested(const QString &pin, const QString &deviceAddress);
#endif

    private:
        bool disconnectCurrentDevice();

        bool connectToDevice(const Model::BluetoothDevice &device);

        bool doConnectToPairedDevice(Model::BluetoothDevice device);

        bool doRemovePair(const Model::BluetoothDevice &device);
#ifdef Q_OS_LINUX
        QString getBluezAdapterPath();
#endif

        configuration::IConfiguration::Pointer configuration_;
        QList<Model::BluetoothAdapter> m_adapters;
        int m_activeAdapterIndex = -1;

        QList<Model::BluetoothDevice> m_devices;
        int m_activeDeviceIndex = -1;

        int m_connectedDeviceCount = 0;
        common::Enum::BluetoothConnectionStatus::Value m_bluetoothConnectionStatus =
            common::Enum::BluetoothConnectionStatus::BC_NOT_CONFIGURED;

        bool m_isScanning = false;
        QStringList m_ignoredDevices;

        std::unique_ptr<QBluetoothLocalDevice> localDevice_;
        QBluetoothDeviceDiscoveryAgent *discoveryAgent_;

#if defined(__LINUX__)
        QDBusInterface m_manager;
        BluetoothAgent *m_agent;
#endif
    };
}
