#pragma once
#include<memory>
#include <../cubeone.canbus.receiver/include/main.h>
#include <../cubeone.canbus.receiver/include/receiver/receiver.h>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/ICarConnect.hpp>

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
                    CarConnect(configuration::IConfiguration::Pointer configuration);
                    ~CarConnect() override;
                    void CheckCarRegistered(char *carId);
                    void monitorCarConnect();
                    void stopCarConnect();
                    bool getStatus();
                    TelemetryItem getSnapshot();

                private:
			using std::enable_shared_from_this<CarConnect>::shared_from_this;
                    SharedItem sharedItem;
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
