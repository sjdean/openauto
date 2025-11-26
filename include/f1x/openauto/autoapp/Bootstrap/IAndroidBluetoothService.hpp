#pragma once
#include <memory>
#include <QBluetoothAddress>

namespace f1x::openauto::autoapp::bootstrap {
    class IAndroidBluetoothService {
    public:
        typedef std::shared_ptr<IAndroidBluetoothService> Pointer;

        virtual ~IAndroidBluetoothService() = default;

        virtual bool registerService(int16_t portNumber, const QBluetoothAddress &bluetoothAddress) = 0;

        virtual bool unregisterService() = 0;
    };
}
