#pragma once

#include <f1x/openauto/autoapp/Service/IServiceFactory.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"
#include "f1x/openauto/autoapp/Projection/InputDevice.hpp"
#include "f1x/openauto/autoapp/Projection/QtAudioInput.hpp"
#include "f1x/openauto/autoapp/Projection/QtAudioOutput.hpp"
#include "f1x/openauto/autoapp/Projection/QtVideoOutput.hpp"

namespace f1x::openauto::autoapp::service {
    class ServiceFactory : public IServiceFactory {
    public:
        ServiceFactory(configuration::IConfiguration::Pointer configuration,
            projection::InputDevice::Pointer inputDevice,
            projection::IVideoOutput::Pointer videoOutput,
            projection::IAudioInput::Pointer audioInput,
            projection::IAudioOutput::Pointer audioOutputSystem,
            projection::IAudioOutput::Pointer audioOutputMedia,
            projection::IAudioOutput::Pointer audioOutputGuidance,
            projection::IAudioOutput::Pointer audioOutputTelephony
            );

        ServiceList create(aasdk::messenger::IMessenger::Pointer messenger) override;

    private:
        IService::Pointer createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createGenericNotificationService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createInputService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createMediaBrowserService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createMediaPlaybackStatusService(aasdk::messenger::IMessenger::Pointer messenger);

        void createMediaSinkServices(ServiceList &serviceList, aasdk::messenger::IMessenger::Pointer messenger);

        void createMediaSourceServices(ServiceList &serviceList, aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createNavigationStatusService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createPhoneStatusService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createRadioService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createSensorService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createVendorExtensionService(aasdk::messenger::IMessenger::Pointer messenger);

        IService::Pointer createWifiProjectionService(aasdk::messenger::IMessenger::Pointer messenger);

        ;
        configuration::IConfiguration::Pointer configuration_;
        projection::InputDevice::Pointer inputDevice_;
        projection::IVideoOutput::Pointer videoOutput_;
        projection::IAudioOutput::Pointer audioOutputSystem_;
        projection::IAudioOutput::Pointer audioOutputGuidance_;
        projection::IAudioOutput::Pointer audioOutputTelephony_;
        projection::IAudioOutput::Pointer audioOutputMedia_;
        projection::IAudioInput::Pointer audioInput_;
    };
}
