#pragma once
#include <QtDBus/QDBusReply>
#include <QtCore/qtimer.h>
#include <f1x/openauto/autoapp/UI/Model/BluetoothDevice.hpp>
#include <f1x/openauto/autoapp/UI/Model/BluetoothAdapter.hpp>
#include <f1x/openauto/autoapp/UI/Enum/BluetoothConnectionStatus.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

#include "f1x/openauto/autoapp/UI/BluetoothAgent.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {
    class BluetoothHandler : public QObject {
        Q_OBJECT

        Q_PROPERTY(
            QVariantList bluetoothAdapterList READ getBluetoothAdapterList NOTIFY
            bluetoothAdapterListChanged)
        Q_PROPERTY(
            QVariantList unpairedDeviceList READ getUnpairedDeviceList NOTIFY unpairedDeviceListChanged)
        Q_PROPERTY(QVariantList pairedDeviceList READ getPairedDeviceList NOTIFY pairedDeviceListChanged)
        Q_PROPERTY(int connectedDeviceCount READ getConnectedDeviceCount NOTIFY connectedDeviceCountChanged)
        Q_PROPERTY(int adapterCount READ getAdapterCount NOTIFY adapterCountChanged)
        Q_PROPERTY(int activeDeviceIndex READ getActiveDeviceIndex NOTIFY activeDeviceIndexChanged)
        Q_PROPERTY(
            Enum::BluetoothConnectionStatus::Value bluetoothConnectionStatus READ getBluetoothConnectionStatus NOTIFY
            bluetoothConnnectionStatusChanged)
        Q_PROPERTY(QObject* agent READ getAgent CONSTANT)

    public:
        explicit BluetoothHandler(configuration::IConfiguration::Pointer configuration,
                                  QObject *parent = nullptr);

        QVariantList getBluetoothAdapterList();

        int getConnectedDeviceCount() const;

        int getAdapterCount() const;

        int getActiveDeviceIndex() const;

        Enum::BluetoothConnectionStatus::Value getBluetoothConnectionStatus() const;

        void setBluetoothConnectionStatus(Enum::BluetoothConnectionStatus::Value value);

        QVariantList getPairedDeviceList();

        QVariantList getUnpairedDeviceList();

        QObject* getAgent() const { return m_agent; }

    public slots:
        Q_INVOKABLE bool doConnectToPairedDevice(const QString &address);

        Q_INVOKABLE bool doRemovePair(const QString &address);

        Q_INVOKABLE bool doRemoveAllPairs();

        Q_INVOKABLE bool doScanDevicesForPairing();

    signals:
        void bluetoothAdapterListChanged();

        void unpairedDeviceListChanged();

        void pairedDeviceListChanged();

        void bluetoothConnnectionStatusChanged();

        void connectedDeviceCountChanged();

        void adapterCountChanged();

        void activeDeviceIndexChanged();

    private slots:
        void onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties);
        void onDeviceFound(const QDBusObjectPath &objectPath, const QVariantMap &interfacesAndProperties);
        void onDeviceConnected(const QDBusObjectPath &path);
        void onDeviceDisconnected(const QDBusObjectPath &path);

    private:
        bool disconnectCurrentDevice();
        bool connectToDevice(const Model::BluetoothDevice &device);
        bool doConnectToPairedDevice(Model::BluetoothDevice device);
        bool doRemovePair(const Model::BluetoothDevice &device);

        f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;

        QDBusInterface m_manager;
        QList<Model::BluetoothAdapter> m_adapters;
        int m_activeAdapterIndex = -1;

        QList<Model::BluetoothDevice> m_devices;
        int m_activeDeviceIndex = -1;

        int m_connectedDeviceCount = 0;
        Enum::BluetoothConnectionStatus::Value m_bluetoothConnectionStatus;

        BluetoothAgent* m_agent;
    };
}
