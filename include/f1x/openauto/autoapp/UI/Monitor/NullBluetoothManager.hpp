#pragma once

#include "f1x/openauto/autoapp/UI/Monitor/IBluetoothManager.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {

    class NullBluetoothManager : public IBluetoothManager {
        Q_OBJECT

    public:
        explicit NullBluetoothManager(QObject *parent = nullptr) : IBluetoothManager(parent) {}

        QVariantList getBluetoothAdapterList() override { return {}; }
        QVariantList getPairedDeviceList() override { return {}; }
        QVariantList getUnpairedDeviceList() override { return {}; }
        common::Enum::BluetoothConnectionStatus::Value getBluetoothConnectionStatus() const override {
            return common::Enum::BluetoothConnectionStatus::BC_NOT_CONFIGURED;
        }
        QString getStatusText() const override { return QStringLiteral("Not available"); }
        bool isScanning() const override { return false; }
        int getConnectedDeviceCount() const override { return 0; }
        int getAdapterCount() const override { return 0; }
        int getActiveDeviceIndex() const override { return -1; }
        QObject *getAgent() const override { return nullptr; }

        QString getAdapterAddress() const override { return {}; }
        void setActiveAdapter(const QString &) override {}
        void startScan() override {}
        void pair(const QString &) override {}
        bool connectToDevice(const QString &) override { return false; }
        void disconnectDevice(const QString &) override {}
        bool removePair(const QString &) override { return false; }
        bool removeAllPairs() override { return false; }
        void ignoreDevice(const QString &) override {}
    };

} // namespace
