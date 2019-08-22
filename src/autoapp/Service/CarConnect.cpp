
#include <thread>
#include <f1x/openauto/autoapp/Service/CarConnect.hpp>
#include <../cubeone.canbus.receiver/include/receiver/receiver.h>

//TODO: Include our C stuff here
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
                        int iDay = 0;
                        int iNight = 0;
                        readOptions(&carId, &dayBrightness, &nightBrightness);

                        dayBrightness = iDay;
                        nightBrightness = iNight;
			sharedItem = initialiseSharedItem(carId, dayBrightness, nightBrightness);
                        isMonitoring = false;
                        CheckCarRegistered(carId);
                    }

                    void CarConnect::CheckCarRegistered(char *carId) {
                        int status = checkRegistered(carId);
                        switch (status) {
                            case REGISTER_NOT_ADOPTED:
                                isRegistered = true;
                                isAdopted = false;
                                break;
                            case REGISTER_OK:
                                isRegistered = true;
                                isAdopted = true;
                                break;
                            default:
                                isRegistered = false;
                                isAdopted = false;
                                break;
                        }
                    }

                    void CarConnect::stopCarConnect() {
                        isMonitoring = false;
                        gps_listen.detach();
                        udp_listen.detach();
                        gps_log.detach();
                        journey_monitor.detach();
                        journey_queue.detach();
                        journey_queue_update.detach();
                    }

                    bool CarConnect::getStatus() {
                        return isMonitoring;
                    }

                    TelemetryItem CarConnect::getSnapshot() {
                        return sharedItem.snapshot;
                    }

                    void CarConnect::monitorCarConnect() {
                        // Our C code will handle passing everything through to Firebase and monitoring
                        // We will take a nominal peek at data (eg speed, fuel, lights, day or night and report back)

                        isMonitoring = true;

                        pthread_t glit;
                        pthread_create(&glit, NULL, gpsListener, (void *) &sharedItem);

 //                       std::thread udp_listen(udpListener, std::ref(sharedItem), 8888);
  //                      std::thread gps_log(gpsLogger, sharedItem);
   //                     std::thread journey_monitor(journeyMonitor, sharedItem);
    //                    std::thread journey_queue(journeyQueue, sharedItem);
     //                   std::thread journey_queue_update(journeyQueueUpdate, sharedItem);

                    }
                }
            }
        }
    }
