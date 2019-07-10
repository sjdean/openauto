
#include <thread>
#include <f1x/openauto/autoapp/cubeone/cubeone.hpp>
#include <cubeone-canbus-receiver/include/canbus-diagnostics-receiver.h>

//TODO: Include our C stuff here
namespace f1x
{
    namespace f1x
    {
        namespace openauto
        {
            namespace autoapp
            {
                namespace service
                {
                    // TODO: We need to add a call back to the C program so we can collect real time information - ie Vol Up/Down etc, Lights On/Off etc
                    CarConnect::CarConnect(configuration::IConfiguration::Pointer configuration)
                            : configuration_(std::move(configuration))

                    {
                        //TODO: Add Remaining Parameters Here
                        char *carId;
                        int iDay;
                        int iNight
                        readOptions(&carId, &dayBrightness, &nightBrightness);

                        this->dayBrightness = iDay;
                        this->nightBrightness = iNight;
                        sharedItem = initialiseSharedItem();
                        isMonitoring = false;
                        this->getCarConnectRegistrationStatus(carId);
                    }

                    void CarConnect::CheckCarRegistered(char *carId) {
                        int status = checkRegistered(carId);
                        switch (status) {
                            case REGISTER_NOT_ADOPTED:
                                this->isRegistered = true;
                                this->isAdopted = false;
                                break;
                            case REGISTED_OK:
                                this->isRegistered = true;
                                this->isAdopted = true;
                                break;
                            default:
                                this->isRegistered = false;
                                this->isAdopted = false;
                                break;
                        }
                    }

                    void CarConnect::stopCarConnect() {
                        isMonitoring = false;
                        delete gps_listen;
                        delete udp_listen;
                        delete gps_log;
                        delete journey_monitor;
                        delete journey_queue;
                        delete journey_queue_update;
                    }

                    bool CarConnect::getStatus() {
                        return isMonitoring;
                    }

                    TelemetryItem CarConnect::getSnapshot() {
                        return sharedItem->snapshot;
                    }

                    void CarConnect::monitorCarConnect() {
                        // Our C code will handle passing everything through to Firebase and monitoring
                        // We will take a nominal peek at data (eg speed, fuel, lights, day or night and report back)

                        isMonitoring = true;

                        gps_listen = new thread(gpsListener, (void *) sharedItem&));
                        udp_listen = new thread(udpListener, (void *) sharedItem&, 8888));
                        gps_log = new thread(gpsLogger, (void *) sharedItem&));
                        journey_monitor = new thread(journeyMonitor, (void *) sharedItem&);
                        journey_queue = new thread(journeyQueue, (void *) sharedItem&));
                        journey_queue_update = new thread(journeyQueueUpdate, (void *) sharedItem&));
                    }
                }
            }
        }
    }
}