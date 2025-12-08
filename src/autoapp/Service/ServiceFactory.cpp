#include <QApplication>
#include <aasdk/Channel/MediaSink/Audio/Channel/MediaAudioChannel.hpp>
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
#include <f1x/openauto/autoapp/Projection/OMXVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/RtAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioInput.hpp>
#include <f1x/openauto/autoapp/Projection/InputDevice.hpp>
#include <f1x/openauto/autoapp/Projection/LocalBluetoothDevice.hpp>
#include <f1x/openauto/autoapp/Projection/DummyBluetoothDevice.hpp>

#include "f1x/openauto/autoapp/Service/Bluetooth/BluetoothService.hpp"
#include "f1x/openauto/autoapp/Service/MediaSink/TelephonyAudioService.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp"
#include "f1x/openauto/Common/Enum/AudioOutputType.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcServiceFactory, "journeyos.service.factory")

namespace f1x::openauto::autoapp::service {
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
                                 configuration::IConfiguration::Pointer configuration)
      : ioService_(ioService), configuration_(std::move(configuration)) {

  }

  ServiceList ServiceFactory::create(aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory) << "[ServiceFactory] create()";
    ServiceList serviceList;

    this->createMediaSinkServices(serviceList, messenger);
    this->createMediaSourceServices(serviceList, messenger);
    serviceList.emplace_back(this->createSensorService(messenger));

    // TODO: as noted elsewhere, this just indiscriminately tries to create a bluetooth service even if we have no device.
    // TODO: This probably is a little crude and should probably be conditional
    serviceList.emplace_back(this->createBluetoothService(messenger));
    serviceList.emplace_back(this->createInputService(messenger));
    serviceList.emplace_back(this->createWifiProjectionService(messenger));

    return serviceList;
  }

  IService::Pointer ServiceFactory::createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory) << "[ServiceFactory] createBluetoothService()";

    projection::IBluetoothDevice::Pointer bluetoothDevice;
    // TODO: This is possibly a little messy. I mean, we fundamentally don't want to create a bluetooth service if we don't have a bluetooth device
    if (configuration_->getSettingByName<QString>("Bluetooth", "BluetoothAdapterAddress") == "") {
      qInfo(lcServiceFactory) <<"[ServiceFactory] Using Dummy Bluetooth";
      bluetoothDevice = std::make_shared<projection::DummyBluetoothDevice>();
    } else {
      qInfo(lcServiceFactory) << "[ServiceFactory] Using Local Bluetooth Adapter";

      bluetoothDevice = projection::IBluetoothDevice::Pointer(new projection::LocalBluetoothDevice(configuration_->getSettingByName<QString>("Bluetooth", "BluetoothAdapterAddress")),
                                                              std::bind(&QObject::deleteLater,
                                                                        std::placeholders::_1));
    }

    return std::make_shared<bluetooth::BluetoothService>(ioService_, messenger, std::move(bluetoothDevice));
  }

  IService::Pointer ServiceFactory::createInputService(aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory)<< "[ServiceFactory] createInputService()";
    QRect videoGeometry;
    switch (configuration_->getSettingByName<int>("AndroidAuto", "Resolution")) {
      case aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1280x720:
        qInfo(lcServiceFactory)<< "[ServiceFactory] Resolution 1280x720";
        videoGeometry = QRect(0, 0, 1280, 720);
        break;
      case aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1920x1080:
        qInfo(lcServiceFactory) << "[ServiceFactory] Resolution 1920x1080";
        videoGeometry = QRect(0, 0, 1920, 1080);
        break;
      default:
        qInfo(lcServiceFactory)<< "[ServiceFactory] Resolution 800x480";
        videoGeometry = QRect(0, 0, 800, 480);
        break;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen == nullptr ? QRect(0, 0, 1, 1) : screen->geometry();
    projection::IInputDevice::Pointer inputDevice(
        std::make_shared<projection::InputDevice>(*QApplication::instance(), configuration_,
                                                  std::move(screenGeometry), std::move(videoGeometry)));

    return std::make_shared<inputsource::InputSourceService>(ioService_, messenger, std::move(inputDevice));
  }

  void ServiceFactory::createMediaSinkServices(ServiceList &serviceList,
                                               aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory)<< "[ServiceFactory] createMediaSinkServices()";
    if (configuration_->getSettingByName<bool>("AndroidAuto", "Media")) {
      qInfo(lcServiceFactory)<< "[ServiceFactory] Media Audio Channel enabled";
      auto mediaAudioOutput =
          static_cast<f1x::openauto::common::Enum::AudioOutputType::Value>(configuration_->getSettingByName<int>("Audio", "Type")) == f1x::openauto::common::Enum::AudioOutputType::RTAUDIO ?
          std::make_shared<projection::RtAudioOutput>(2, 16, 48000) :
          projection::IAudioOutput::Pointer(new projection::QtAudioOutput(2, 16, 48000),
                                            std::bind(&QObject::deleteLater, std::placeholders::_1));

      serviceList.emplace_back(
          std::make_shared<mediasink::MediaAudioService>(ioService_, messenger, std::move(mediaAudioOutput)));
    }

    if (configuration_->getSettingByName<bool>("AndroidAuto", "Guidance")) {
      qInfo(lcServiceFactory)<< "[ServiceFactory] Guidance Audio Channel enabled";
      auto guidanceAudioOutput =
          static_cast<f1x::openauto::common::Enum::AudioOutputType::Value>(configuration_->getSettingByName<int>("Audio", "Type")) == f1x::openauto::common::Enum::AudioOutputType::RTAUDIO ?
          std::make_shared<projection::RtAudioOutput>(1, 16, 16000) :
          projection::IAudioOutput::Pointer(new projection::QtAudioOutput(1, 16, 16000),
                                            std::bind(&QObject::deleteLater, std::placeholders::_1));

      serviceList.emplace_back(
          std::make_shared<mediasink::GuidanceAudioService>(ioService_, messenger,
                                                            std::move(guidanceAudioOutput)));
    }

    if (configuration_->getSettingByName<bool>("AndroidAuto", "Telephony")) {
      qInfo(lcServiceFactory)<< "[ServiceFactory] Telephony Audio Channel enabled";
      auto telephonyAudioOutput =
          static_cast<f1x::openauto::common::Enum::AudioOutputType::Value>(configuration_->getSettingByName<int>("Audio", "Type")) == f1x::openauto::common::Enum::AudioOutputType::RTAUDIO ?
          std::make_shared<projection::RtAudioOutput>(1, 16, 16000) :
          projection::IAudioOutput::Pointer(new projection::QtAudioOutput(1, 16, 16000),
                                            std::bind(&QObject::deleteLater, std::placeholders::_1));

      serviceList.emplace_back(
          std::make_shared<mediasink::TelephonyAudioService>(ioService_, messenger,
                                                             std::move(telephonyAudioOutput)));
    }

    /*
     * No Need to Check for systemAudioChannelEnabled - MUST be enabled by default.
     */

    qInfo(lcServiceFactory)<< "[ServiceFactory] System Audio Channel enabled";
    auto systemAudioOutput =
        static_cast<f1x::openauto::common::Enum::AudioOutputType::Value>(configuration_->getSettingByName<int>("Audio", "Type")) == f1x::openauto::common::Enum::AudioOutputType::RTAUDIO ?
        std::make_shared<projection::RtAudioOutput>(1, 16, 16000) :
        projection::IAudioOutput::Pointer(new projection::QtAudioOutput(1, 16, 16000),
                                          std::bind(&QObject::deleteLater, std::placeholders::_1));

    serviceList.emplace_back(
        std::make_shared<mediasink::SystemAudioService>(ioService_, messenger, std::move(systemAudioOutput)));

    // TODO: What is OMX??? ensure we're cross compiled to allow Mac, Windows and Linux. Happy for stub for Windows for now.
#ifdef USE_OMX
    auto videoOutput(std::make_shared<projection::OMXVideoOutput>(configuration_));
#else
    projection::IVideoOutput::Pointer videoOutput(new projection::QtVideoOutput(configuration_),
                                                  std::bind(&QObject::deleteLater, std::placeholders::_1));
#endif

    qInfo(lcServiceFactory)<< "[ServiceFactory] Video Channel enabled";
    serviceList.emplace_back(
        std::make_shared<mediasink::VideoService>(ioService_, messenger, std::move(videoOutput)));
  }

  void ServiceFactory::createMediaSourceServices(f1x::openauto::autoapp::service::ServiceList &serviceList,
                                                 aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory)<< "[ServiceFactory] createMediaSourceServices()";
    projection::IAudioInput::Pointer audioInput(new projection::QtAudioInput(1, 16, 16000),
                                                std::bind(&QObject::deleteLater, std::placeholders::_1));
    serviceList.emplace_back(std::make_shared<mediasource::MicrophoneMediaSourceService>(ioService_, messenger,
                                                                                         std::move(audioInput)));
  }

  IService::Pointer ServiceFactory::createSensorService(aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory)<< "[ServiceFactory] createSensorService()";
    return std::make_shared<sensor::SensorService>(ioService_, messenger);
  }

  IService::Pointer ServiceFactory::createWifiProjectionService(aasdk::messenger::IMessenger::Pointer messenger) {
    qInfo(lcServiceFactory)<< "[ServiceFactory] createWifiProjectionService()";
    return std::make_shared<wifiprojection::WifiProjectionService>(ioService_, messenger, configuration_);
  }

}



