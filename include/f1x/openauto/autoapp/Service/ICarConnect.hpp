#pragma once
#include<memory>
#include <../cubeone.canbus.receiver/include/receiver/receiver.h>
namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace service
            {
                class ICarConnect
                {
                public:
                    typedef std::shared_ptr<ICarConnect> Pointer;
                    virtual ~ICarConnect() = default;
                    virtual void CheckCarRegistered(char *carId) = 0;
                    virtual void monitorCarConnect() = 0;
                    virtual void stopCarConnect() = 0;
                    virtual bool getStatus() = 0;
                    virtual TelemetryItem getSnapshot() = 0;
                };
            }
        }
    }
}
