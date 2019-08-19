
#include <thread>
#include <f1x/openauto/autoapp/Service/CarConnect.hpp>

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
                        sharedItem = initialiseSharedItem();
                        isMonitoring = false;
                        getCarConnectRegistrationStatus(carId);
                    }

                    void CarConnect::CheckCarRegistered(char *carId) {
                        int status = checkRegistered(carId);
                        switch (status) {
                            case REGISTER_NOT_ADOPTED:
                                isRegistered = true;
                                isAdopted = false;
                                break;
                            case REGISTED_OK:
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

                        gps_listen = new std::thread(gpsListener, (void *) sharedItem&));
                        udp_listen = new std::thread(udpListener, (void *) sharedItem&, 8888));
                        gps_log = new std::thread(gpsLogger, (void *) sharedItem&));
                        journey_monitor = new std::thread(journeyMonitor, (void *) sharedItem&);
                        journey_queue = new std::thread(journeyQueue, (void *) sharedItem&));
                        journey_queue_update = new std::thread(journeyQueueUpdate, (void *) sharedItem&));
                    }
                }
            }
        }
    }
