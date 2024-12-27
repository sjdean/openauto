

#pragma once#include <QtBluetooth/QBluetoothServiceInfo>
#include <f1x/openauto/btservice/IAndroidBluetoothService.hpp>

namespace f1x::openauto::btservice
{

class AndroidBluetoothService: public IAndroidBluetoothService
{
public:
    AndroidBluetoothService();

    bool registerService(int16_t portNumber, const QBluetoothAddress& bluetoothAddress) override;
    bool unregisterService() override;

private:
    QBluetoothServiceInfo serviceInfo_;
};

}


