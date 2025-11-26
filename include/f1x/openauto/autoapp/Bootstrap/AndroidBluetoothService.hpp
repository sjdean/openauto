#pragma once
#include <QtBluetooth/QBluetoothServiceInfo>
#include <f1x/openauto/autoapp/Bootstrap/IAndroidBluetoothService.hpp>

namespace f1x::openauto::autoapp::bootstrap {
    class AndroidBluetoothService : public IAndroidBluetoothService {
    public:
        AndroidBluetoothService();

        bool registerService(int16_t portNumber, const QBluetoothAddress &bluetoothAddress) override;

        bool unregisterService() override;

    private:
        QBluetoothServiceInfo serviceInfo_;
    };
}
