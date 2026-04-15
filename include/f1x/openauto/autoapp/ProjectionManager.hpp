#pragma once

#include <QTcpServer>
#include <aasdk/USB/IUSBHub.hpp>
#include <aasdk/USB/IConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/USBWrapper.hpp>
#include <aasdk/TCP/ITCPEndpoint.hpp>
#include <aasdk/TCP/QtTCPEndpoint.hpp>

// Core Interfaces
#include <f1x/openauto/autoapp/Service/IAndroidAutoSessionEventHandler.hpp>
#include <f1x/openauto/autoapp/Service/ISessionFactory.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

// Bootstrap / Bluetooth
#include "Bootstrap/AndroidBluetoothService.hpp"
#include "Bootstrap/IAndroidBluetoothServer.hpp"
#include "Bootstrap/IAndroidBluetoothService.hpp"
#include "Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp {
    class ProjectionManager :
            public service::IAndroidAutoSessionEventHandler,
            public std::enable_shared_from_this<ProjectionManager> {
    public:
        typedef std::shared_ptr<ProjectionManager> Pointer;

        ProjectionManager(configuration::IConfiguration::Pointer configuration,
            aasdk::usb::USBWrapper &usbWrapper,
            aasdk::tcp::ITCPWrapper &tcpWrapper,
            service::ISessionFactory &sessionFactory,
            aasdk::usb::IUSBHub::Pointer usbHub,
            aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator,
            std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor
        );

        void waitForUSBDevice();

        void start(aasdk::tcp::ITCPEndpoint::Pointer endpoint);

        void stop();

        void pause();

        void resume();

        void onAndroidAutoQuit() override;

        bool disableAutostartEntity = false;

    private:
        using std::enable_shared_from_this<ProjectionManager>::shared_from_this;

        void enumerateDevices();
        void waitForDevice();
        void aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle);
        void onUSBHubError(const aasdk::error::Error &error);
        void handleNewClient();

        // Core Infrastructure
        ;
        aasdk::usb::USBWrapper &usbWrapper_;
        QTcpServer tcpServer_;

        // Factory & Hardware
        service::ISessionFactory &sessionFactory_;
        aasdk::usb::IUSBHub::Pointer usbHub_;
        aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator_;

        // Active Session (The current phone connection)
        service::IAndroidAutoSession::Pointer activeSession_;

        // UI & Config
        std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor_;
        bootstrap::IAndroidBluetoothServer::Pointer androidBluetoothServer_;
        bootstrap::IAndroidBluetoothService::Pointer androidBluetoothService_;
        configuration::IConfiguration::Pointer configuration_;

        bool isStopped_;
    };
}