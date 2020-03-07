#pragma once
#include<memory>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/ICarConnect.hpp>
#include <uk/co/cubeone/canbus-receiver/definitions/Definitions.h>

namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace service
            {
                class CarConnect: public ICarConnect, public std::enable_shared_from_this<CarConnect>
                {
                public:
                    explicit CarConnect(configuration::IConfiguration::Pointer configuration);
                    uk::co::cubeone::definitions::TelemetryItem getSnapshot() override;

                private:
                    using std::enable_shared_from_this<CarConnect>::shared_from_this;
                    configuration::IConfiguration::Pointer configuration_;

                    std::thread gps_listen;
                    std::thread udp_listen;
                    std::thread gps_log;
                    std::thread journey_monitor;
                    std::thread journey_queue;
                    std::thread journey_queue_update;
                };
            }
        }
    }
}
