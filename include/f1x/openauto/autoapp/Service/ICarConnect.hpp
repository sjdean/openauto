#pragma once
#include<memory>
#include<uk/co/cubeone/canbus-receiver/definitions/Definitions.h>
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
                    virtual uk::co::cubeone::definitions::TelemetryItem getSnapshot() = 0;
                };
            }
        }
    }
}
