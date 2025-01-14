#pragma once

#include <f1x/openauto/autoapp/Projection/IBluetoothDevice.hpp>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceInfo>
#include <QtBluetooth/QBluetoothServer>

namespace f1x::openauto::autoapp::projection
{

class LocalBluetoothDevice: public QObject, public IBluetoothDevice
{
    Q_OBJECT

public:
    LocalBluetoothDevice(const QString &adapterAddress = QString(), QObject *parent = nullptr);

    void stop() override;
    bool isPaired(const std::string& address) const override;
    std::string getAdapterAddress() const override;
    bool isAvailable() const override;

private slots:
    void createBluetoothLocalDevice(const QString &adapterAddress);

private:
    mutable std::mutex mutex_;
    std::unique_ptr<QBluetoothLocalDevice> localDevice_;
    PairingPromise::Pointer pairingPromise_;
    QBluetoothAddress pairingAddress_;
    QBluetoothServiceInfo serviceInfo_;
    std::unique_ptr<QBluetoothServer> rfcommServer_;

};

}



