#include <aasdk/IO/Promise.hpp>

#pragma once

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class IBluetoothDevice
{
public:
    typedef aasdk::io::Promise<void, void> PairingPromise;
    typedef std::shared_ptr<IBluetoothDevice> Pointer;

    virtual void stop() = 0;
    virtual bool isPaired(const std::string& address) const = 0;
    virtual std::string getAdapterAddress() const = 0;
    virtual bool isAvailable() const = 0;
};

}
}
}
}
