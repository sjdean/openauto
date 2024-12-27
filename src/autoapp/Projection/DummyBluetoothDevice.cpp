#include <f1x/openauto/autoapp/Projection/DummyBluetoothDevice.hpp>




namespace f1x::openauto::autoapp::projection
{

void DummyBluetoothDevice::stop()
{

}

bool DummyBluetoothDevice::isPaired(const std::string&) const
{
    return false;
}

std::string DummyBluetoothDevice::getAdapterAddress() const
{
    return "";
}

bool DummyBluetoothDevice::isAvailable() const
{
    return false;
}

}



