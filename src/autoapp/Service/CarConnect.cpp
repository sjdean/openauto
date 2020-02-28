
#include <thread>
#include <f1x/openauto/autoapp/Service/CarConnect.hpp>
#include <uk/co/cubeone/canbus-receiver/service/CarHandler.h>
#include <uk/co/cubeone/canbus-receiver/service/SnapShotHandler.h>

    namespace f1x
    {
        namespace openauto
        {
            namespace autoapp
            {
                namespace service
                {
                    CarConnect::CarConnect(configuration::IConfiguration::Pointer configuration)
                            : configuration_(std::move(configuration))

                    {
                        uk::co::cubeone::service::CarHandler ch = uk::co::cubeone::service::CarHandler(configuration_->getCarKey());
			            // TODO: Do we need to do this?
                        //sharedItem = initialiseSharedItem(configuration_->getCarKey(), configuration_->getDayBrightness(), configuration_->getNightBrightness());

                        uk::co::cubeone::definitions::RegistrationStatus status = ch.getRegistrationStatus();
                        if (status == uk::co::cubeone::definitions::RegistrationStatus::OK) {
                            ch.start();
                        }
                    }

                    uk::co::cubeone::definitions::TelemetryItem CarConnect::getSnapshot() {
                        return uk::co::cubeone::service::SnapShotHandler().getSnapShot();
                    }
                }
            }
        }
    }
