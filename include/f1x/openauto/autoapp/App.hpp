#pragma once

#include <aasdk/USB/IUSBHub.hpp>
#include <aasdk/USB/IConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/USBWrapper.hpp>
#include <aasdk/TCP/ITCPWrapper.hpp>
#include <aasdk/TCP/ITCPEndpoint.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityEventHandler.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

#include "Bootstrap/AndroidBluetoothService.hpp"
#include "Bootstrap/IAndroidBluetoothServer.hpp"
#include "Bootstrap/IAndroidBluetoothService.hpp"
#include "Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp {
    class App :
            public service::IAndroidAutoEntityEventHandler,
            public std::enable_shared_from_this<App> {
    public:
        typedef std::shared_ptr<App> Pointer;

        App(configuration::IConfiguration::Pointer configuration,
            boost::asio::io_service &ioService,
            aasdk::usb::USBWrapper &usbWrapper,
            aasdk::tcp::ITCPWrapper &tcpWrapper,
            service::IAndroidAutoEntityFactory &androidAutoEntityFactory,
            aasdk::usb::IUSBHub::Pointer usbHub,
            aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator,
            std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor
        );

        void waitForUSBDevice();

        void start(aasdk::tcp::ITCPEndpoint::SocketPointer socket);

        void stop();

        void pause();

        void resume();

        void onAndroidAutoQuit() override;

        bool disableAutostartEntity = false;

    private:
        using std::enable_shared_from_this<App>::shared_from_this;

        void enumerateDevices();

        void waitForDevice();

        void aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle);

        void onUSBHubError(const aasdk::error::Error &error);

        boost::asio::io_service &ioService_;
        aasdk::usb::USBWrapper &usbWrapper_;
        aasdk::tcp::ITCPWrapper &tcpWrapper_;
        boost::asio::ip::tcp::acceptor acceptor_;
        boost::asio::io_service::strand strand_;
        service::IAndroidAutoEntityFactory &androidAutoEntityFactory_;
        aasdk::usb::IUSBHub::Pointer usbHub_;
        aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator_;
        service::IAndroidAutoEntity::Pointer androidAutoEntity_;
        std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor_;
        bootstrap::IAndroidBluetoothServer::Pointer androidBluetoothServer_;
        bootstrap::IAndroidBluetoothService::Pointer androidBluetoothService_;
        configuration::IConfiguration::Pointer configuration_;

        bool isStopped_;

        void startServerSocket();

        void handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                             const boost::system::error_code &err);
    };
}
