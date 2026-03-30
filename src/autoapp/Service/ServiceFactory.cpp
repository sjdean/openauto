#include <QApplication>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoService.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/MediaAudioService.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/GuidanceAudioService.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/SystemAudioService.hpp>
#include <f1x/openauto/autoapp/Service/MediaSource/MicrophoneMediaSourceService.hpp>
#include <f1x/openauto/autoapp/Service/Sensor/SensorService.hpp>
#include <f1x/openauto/autoapp/Service/InputSource/InputSourceService.hpp>
#include <f1x/openauto/autoapp/Service/WifiProjection/WifiProjectionService.hpp>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioInput.hpp>
#include <f1x/openauto/autoapp/Projection/InputDevice.hpp>
#include <f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp>
#include <f1x/openauto/autoapp/Projection/DummyBluetoothDevice.hpp>
#include "f1x/openauto/autoapp/Service/Bluetooth/BluetoothService.hpp"
#include "f1x/openauto/autoapp/Service/MediaSink/TelephonyAudioService.hpp"
#include "f1x/openauto/autoapp/Service/NavigationStatus/NavigationStatusService.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp"

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceFactory, "journeyos.service.factory")

namespace f1x::openauto::autoapp::service {
using configuration::ConfigGroup;
using configuration::ConfigKey;
    /**
     * @class ServiceFactory
     * @brief A factory class responsible for creating and managing instances of various services.
     *
     * This class provides mechanisms to instantiate, configure, and manage lifecycle of services
     * within the application. It serves as an abstraction layer that simplifies the creation and
     * retrieval of service objects, ensuring consistency and reducing tight coupling between components.
     *
     * The ServiceFactory is typically used to handle dependency injection, service discovery,
     * or configuring and returning reusable service singletons or scoped service instances.
     *
     * Responsibilities of the ServiceFactory include:
     * - Managing the instantiation logic for services.
     * - Optionally supporting service configuration or dependency resolution.
     * - Providing methods to retrieve service instances.
     * - Ensuring services are properly disposed of when no longer needed (if applicable).
     *
     * Thread safety of this class depends on its implementation. If the ServiceFactory
     * is intended to be shared across threads, the implementation should ensure concurrent
     * safe access to service instances.
     *
     * Depending on the use case, the ServiceFactory can support various service lifetimes:
     * - Singleton: One instance shared globally.
     * - Transient: A new instance provided each time.
     * - Scoped: Instances scoped to a certain context.
     *
     * Note that this class assumes responsibility for ensuring efficient use of resources
     * and clean lifecycle management of the services it creates.
     */
    ServiceFactory::ServiceFactory(boost::asio::io_service &ioService,
                                   configuration::IConfiguration::Pointer configuration,
                                   projection::InputDevice::Pointer inputDevice,
                                   projection::IVideoOutput::Pointer videoOutput,
                                   projection::IAudioInput::Pointer audioInput,
                                   projection::IAudioOutput::Pointer audioOutputSystem,
                                   projection::IAudioOutput::Pointer audioOutputMedia,
                                   projection::IAudioOutput::Pointer audioOutputGuidance,
                                   projection::IAudioOutput::Pointer audioOutputTelephony)
        : ioService_(ioService)
          , configuration_(std::move(configuration))
          , inputDevice_(inputDevice)
          , videoOutput_(videoOutput)
          , audioOutputSystem_(audioOutputSystem)
          , audioOutputGuidance_(audioOutputGuidance)
          , audioOutputTelephony_(audioOutputTelephony)
          , audioOutputMedia_(audioOutputMedia)
          , audioInput_(audioInput) {
    }

    ServiceList ServiceFactory::create(aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << ">>> building service list";
        ServiceList serviceList;

        this->createMediaSinkServices(serviceList, messenger);
        this->createMediaSourceServices(serviceList, messenger);
        serviceList.emplace_back(this->createSensorService(messenger));

        serviceList.emplace_back(this->createBluetoothService(messenger));
        serviceList.emplace_back(this->createInputService(messenger));
        //serviceList.emplace_back(this->createNavigationStatusService(messenger));
        //serviceList.emplace_back(this->createWifiProjectionService(messenger));

        return serviceList;
    }

    IService::Pointer ServiceFactory::createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << "creating bluetooth service";

        projection::IBluetoothDevice::Pointer bluetoothDevice;
        // TODO: This is possibly a little messy. I mean, we fundamentally don't want to create a bluetooth service if we don't have a bluetooth device
        if (configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress) == "") {
            qInfo(lcServiceFactory) << "bluetooth=dummy (no adapter address configured)";
            bluetoothDevice = std::make_shared<projection::DummyBluetoothDevice>();
        } else {
            qInfo(lcServiceFactory) << "bluetooth=local";

            bluetoothDevice = projection::IBluetoothDevice::Pointer(
                new projection::LocalBluetoothDevice(
                    configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress)),
                std::bind(&QObject::deleteLater,
                          std::placeholders::_1));
        }

        return std::make_shared<bluetooth::BluetoothService>(ioService_, messenger, std::move(bluetoothDevice));
    }

    IService::Pointer ServiceFactory::createInputService(aasdk::messenger::IMessenger::Pointer messenger) {


        return std::make_shared<inputsource::InputSourceService>(ioService_, messenger, inputDevice_);
    }

    void ServiceFactory::createMediaSinkServices(ServiceList &serviceList,
                                                 aasdk::messenger::IMessenger::Pointer messenger) {
        if (configuration_->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoMedia)) {
            if (audioOutputMedia_) {
                qInfo(lcServiceFactory) << "sink registered channel=media";
                serviceList.emplace_back(
                    std::make_shared<mediasink::MediaAudioService>(ioService_, messenger, audioOutputMedia_));
            }
        }

        if (configuration_->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoGuidance)) {
            if (audioOutputGuidance_) {
                qInfo(lcServiceFactory) << "sink registered channel=guidance";
                serviceList.emplace_back(
                    std::make_shared<mediasink::GuidanceAudioService>(ioService_, messenger, audioOutputGuidance_));
            }
        }
/*
        if (configuration_->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoTelephony)) {
            if (audioOutputTelephony_) {
                qInfo(lcServiceFactory) << "......Telephony channel";
                serviceList.emplace_back(
                    std::make_shared<mediasink::TelephonyAudioService>(ioService_, messenger, audioOutputTelephony_));
            }
        }
*/
        /*
         * No Need to Check for systemAudioChannelEnabled - MUST be enabled by default.
         */

        qInfo(lcServiceFactory) << "sink registered channel=system";

        serviceList.emplace_back(
            std::make_shared<mediasink::SystemAudioService>(ioService_, messenger, audioOutputSystem_));


        qInfo(lcServiceFactory) << "sink registered channel=video";
        serviceList.emplace_back(
            std::make_shared<mediasink::VideoService>(ioService_, messenger, videoOutput_));
    }

    void ServiceFactory::createMediaSourceServices(f1x::openauto::autoapp::service::ServiceList &serviceList,
                                                   aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << "source registered channel=microphone";
        // Use the pre-initialised audio input passed to this factory rather than
        // creating a new device on every session.
        serviceList.emplace_back(std::make_shared<mediasource::MicrophoneMediaSourceService>(ioService_, messenger,
            audioInput_));
    }

    IService::Pointer ServiceFactory::createSensorService(aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << "service registered channel=sensors";
        return std::make_shared<sensor::SensorService>(ioService_, messenger);
    }

    IService::Pointer ServiceFactory::createNavigationStatusService(aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << "service registered channel=navigation-status";
        return std::make_shared<navigationstatus::NavigationStatusService>(ioService_, messenger);
    }

    IService::Pointer ServiceFactory::createWifiProjectionService(aasdk::messenger::IMessenger::Pointer messenger) {
        qInfo(lcServiceFactory) << "service registered channel=wifi-projection";
        return std::make_shared<wifiprojection::WifiProjectionService>(ioService_, messenger, configuration_);
    }
}
