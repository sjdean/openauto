#pragma once

#include <QtCore/QObject>
#include <QString>
#include <QVariantList>

#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

    class IBluetoothManager : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool isPairingModeEnabled READ isPairingModeEnabled NOTIFY pairingModeEnabledChanged)

    public:
        explicit IBluetoothManager(QObject *parent = nullptr) : QObject(parent) {}
        ~IBluetoothManager() override = default;

        // Property accessors
        virtual QVariantList getBluetoothAdapterList() = 0;
        virtual QVariantList getPairedDeviceList() = 0;
        virtual QVariantList getUnpairedDeviceList() = 0;
        virtual common::Enum::BluetoothConnectionStatus::Value getBluetoothConnectionStatus() const = 0;
        virtual QString getStatusText() const = 0;
        virtual bool isScanning() const = 0;
        virtual int getConnectedDeviceCount() const = 0;
        virtual int getAdapterCount() const = 0;
        virtual int getActiveDeviceIndex() const = 0;
        virtual QObject *getAgent() const = 0;

        // Invokables
        Q_INVOKABLE virtual QString getAdapterAddress() const = 0;
        Q_INVOKABLE virtual void setActiveAdapter(const QString &address) = 0;
        Q_INVOKABLE virtual void startScan() = 0;
        Q_INVOKABLE virtual void pair(const QString &address) = 0;
        Q_INVOKABLE virtual bool connectToDevice(const QString &address) = 0;
        Q_INVOKABLE virtual void disconnectDevice(const QString &address) = 0;
        Q_INVOKABLE virtual bool removePair(const QString &address) = 0;
        Q_INVOKABLE virtual bool removeAllPairs() = 0;
        Q_INVOKABLE virtual void ignoreDevice(const QString &address) = 0;
        Q_INVOKABLE virtual void enablePairingMode(bool enabled) = 0;

        virtual bool isPairingModeEnabled() const = 0;

    signals:
        void bluetoothAdapterListChanged();
        void unpairedDeviceListChanged();
        void pairedDeviceListChanged();
        void bluetoothConnectionStatusChanged();
        void connectedDeviceCountChanged();
        void adapterCountChanged();
        void activeDeviceIndexChanged();
        void isScanningChanged();
        void pairingPinConfirmation(const QString &pin, const QString &deviceAddress);
        void pairingModeEnabledChanged();
    };

} // namespace
