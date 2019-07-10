#pragma once
#include <cubeone-canbus-receiver/include/canbus-diagnostics-receiver.h>

namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace service
            {

                class CarConnect:
                        public IPinger,
                        public std::enable_shared_from_this<Pinger>
                {
                public:
                    CarConnect(configuration::IConfiguration::Pointer configuration);
                    void CheckCarRegistered(char *carId);
                    void monitorCarConnect();
                    void stopCarConnect();
                    bool getStatus();
                    TelemetryItem getSnapshot();

                private:
                    SharedItem* sharedItem;
                    int32_t dayBrightness;
                    int32_t nightBrightness;
                    bool isMonitoring;
                    bool isRegistered;
                    bool isAdopted;
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
