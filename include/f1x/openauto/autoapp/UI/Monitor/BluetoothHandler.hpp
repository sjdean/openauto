#pragma once
#include <QtCore/QObject>
#include <QTimer>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "f1x/openauto/autoapp/UI/Model/BluetoothDevice.hpp"
#include "f1x/openauto/autoapp/UI/Model/BluetoothAdapter.hpp"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

#ifdef Q_OS_LINUX
#include "f1x/openauto/autoapp/UI/BluetoothAgent.hpp"
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#endif

#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#include "f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/IBluetoothManager.hpp"


namespace f1x::openauto::autoapp::UI::Monitor {
    class BluetoothHandler : public IBluetoothManager {
        Q_OBJECT
        // Properties
        Q_PROPERTY(QVariantList bluetoothAdapterList READ getBluetoothAdapterList NOTIFY bluetoothAdapterListChanged)
        Q_PROPERTY(QVariantList unpairedDeviceList READ getUnpairedDeviceList NOTIFY unpairedDeviceListChanged)
        Q_PROPERTY(QVariantList pairedDeviceList READ getPairedDeviceList NOTIFY pairedDeviceListChanged)
        Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
        Q_PROPERTY(int adapterCount READ getAdapterCount NOTIFY adapterCountChanged)
        Q_PROPERTY(int activeDeviceIndex READ getActiveDeviceIndex NOTIFY activeDeviceIndexChanged)
        Q_PROPERTY(
            f1x::openauto::common::Enum::BluetoothConnectionStatus::Value bluetoothConnectionStatus
            READ getBluetoothConnectionStatus NOTIFY bluetoothConnectionStatusChanged)
        Q_PROPERTY(QString statusText READ getStatusText NOTIFY bluetoothConnectionStatusChanged)
        Q_PROPERTY(QString connectedDeviceName READ getConnectedDeviceName NOTIFY bluetoothConnectionStatusChanged)
        Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
        // Agent property only valid on Linux, or return nullptr on Mac
        Q_PROPERTY(QObject* agent READ getAgent CONSTANT)
        Q_PROPERTY(bool isPairingModeEnabled READ isPairingModeEnabled NOTIFY pairingModeEnabledChanged)

    public:
        explicit BluetoothHandler(configuration::IConfiguration::Pointer configuration,
                                  QObject *parent = nullptr);

        // Public Q_INVOKABLEs (Called by UI)
        Q_INVOKABLE QString getAdapterAddress() const override;

        Q_INVOKABLE void setActiveAdapter(const QString &address) override;

        Q_INVOKABLE void startScan() override;

        Q_INVOKABLE void pair(const QString &address) override;

        Q_INVOKABLE bool connectToDevice(const QString &address) override;
        Q_INVOKABLE void disconnectDevice(const QString &address) override;

        Q_INVOKABLE bool removePair(const QString &address) override;

        Q_INVOKABLE bool removeAllPairs() override;

        Q_INVOKABLE void enablePairingMode(bool enabled) override;

        bool isPairingModeEnabled() const override { return m_pairingModeEnabled; }

        QVariantList getBluetoothAdapterList() override;

        int getConnectedDeviceCount() const override;

        int getAdapterCount() const override;

        int getActiveDeviceIndex() const override;

        common::Enum::BluetoothConnectionStatus::Value getBluetoothConnectionStatus() const override;

        void setBluetoothConnectionStatus(common::Enum::BluetoothConnectionStatus::Value value);

        QString getStatusText() const override;

        QString getConnectedDeviceName() const;

        bool isScanning() const override;

        Q_INVOKABLE void ignoreDevice(const QString &address) override;

        QVariantList getPairedDeviceList() override;

        QVariantList getUnpairedDeviceList() override;

        QObject *getAgent() const override {
#ifdef Q_OS_LINUX
            return m_agent;
#else
            return nullptr;
#endif
        }

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

        bool connectToDeviceImpl(const Model::BluetoothDevice &device);

        bool connectToPairedDeviceImpl(Model::BluetoothDevice device);

        bool removePairImpl(const Model::BluetoothDevice &device);
#ifdef Q_OS_LINUX
        QString getBluezAdapterPath();
        void loadPairedDevicesFromBlueZ();
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
        bool m_pairingModeEnabled = true;
        QStringList m_ignoredDevices;

        std::unique_ptr<QBluetoothLocalDevice> localDevice_;
        QBluetoothDeviceDiscoveryAgent *discoveryAgent_;

#ifdef Q_OS_LINUX
        QDBusInterface m_manager;
        BluetoothAgent *m_agent;
#endif
    };
}
