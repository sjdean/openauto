#include "f1x/openauto/autoapp/ProjectionManager.hpp"

#include <thread>
#include <aasdk/USB/AOAPDevice.hpp>
#include <aasdk/TCP/TCPEndpoint.hpp>
#include <aap_protobuf/service/bluetooth/message/BluetoothAuthenticationData.pb.h>

#include "f1x/openauto/autoapp/Bootstrap/AndroidBluetoothServer.hpp"
#include "f1x/openauto/autoapp/Bootstrap/AndroidBluetoothService.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcProjectionManager, "journeyos.projection.manager")

namespace f1x::openauto::autoapp {
using configuration::ConfigGroup;
using configuration::ConfigKey;

    ProjectionManager::ProjectionManager(configuration::IConfiguration::Pointer configuration,
             boost::asio::io_service &ioService,
             aasdk::usb::USBWrapper &usbWrapper,
             aasdk::tcp::ITCPWrapper &tcpWrapper,
             service::ISessionFactory &sessionFactory,
             aasdk::usb::IUSBHub::Pointer usbHub,
             aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator,
             std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor
    )
        : ioService_(ioService),
          usbWrapper_(usbWrapper),
          tcpWrapper_(tcpWrapper),
          acceptor_(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 5000)),
          strand_(ioService_),
          sessionFactory_(sessionFactory),
          usbHub_(std::move(usbHub)),
          connectedAccessoriesEnumerator_(std::move(connectedAccessoriesEnumerator)),
          androidAutoMonitor_(std::move(androidAutoMonitor)),
          configuration_(configuration),
          isStopped_(false)
    {
        qInfo(lcProjectionManager) << "Initializing ProjectionManager...";
        androidAutoMonitor_->onConnectionStateUpdate(common::Enum::AndroidAutoConnectivityState::AA_STARTUP);

        QString adapterAddress = configuration_->getSettingByName<QString>(ConfigGroup::Bluetooth, ConfigKey::BluetoothAdapterAddress);
        QBluetoothAddress address(adapterAddress);
        androidBluetoothServer_ = std::make_shared<bootstrap::AndroidBluetoothServer>(configuration_);

        uint16_t port = androidBluetoothServer_->start(address);
        if (port > 0) {
            qInfo(lcProjectionManager) << "Bootstrap Server started on port: " << port;

            // Register Service (Advertise UUID so phone finds us)
            androidBluetoothService_ = std::make_shared<bootstrap::AndroidBluetoothService>();
            if(androidBluetoothService_->registerService(port, QBluetoothAddress())) {
                qInfo(lcProjectionManager) << "Bootstrap Service Registered successfully.";
            } else {
                qCritical(lcProjectionManager) << "Failed to register Bootstrap Service.";
            }
        } else {
            qCritical(lcProjectionManager) << "Failed to start Bootstrap Server.";
        }
    }

    void ProjectionManager::waitForUSBDevice() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            try {
                qInfo(lcProjectionManager) << "Waiting for USB Device...";
                this->waitForDevice();
            } catch (...) {
                qCritical(lcProjectionManager) << "Exception in waitForDevice()";
            }
            try {
                this->enumerateDevices();
            } catch (...) {
                qCritical(lcProjectionManager) << "Exception in enumerateDevices()";
            }
        });
    }

    void ProjectionManager::start(aasdk::tcp::ITCPEndpoint::SocketPointer socket) {
        strand_.dispatch([this, self = this->shared_from_this(), socket = std::move(socket)]() mutable {
            qInfo(lcProjectionManager) << "Starting Wireless Session (TCP)";

            if (activeSession_ != nullptr) {
                try {
                    qInfo(lcProjectionManager) << "Stopping existing session first.";
                    activeSession_->stop();
                    activeSession_.reset();
                } catch (...) {
                    qCritical(lcProjectionManager) << "Error stopping previous session.";
                }
            }

            try {
                auto tcpEndpoint(std::make_shared<aasdk::tcp::TCPEndpoint>(tcpWrapper_, std::move(socket)));
                activeSession_ = sessionFactory_.create(std::move(tcpEndpoint));
                activeSession_->start(*this);
            } catch (const aasdk::error::Error &error) {
                qCritical(lcProjectionManager) << "Failed to create Wireless Session: " << error.what();
                this->waitForDevice();
            }
        });
    }

    void ProjectionManager::stop() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            isStopped_ = true;
            try {
                connectedAccessoriesEnumerator_->cancel();
                usbHub_->cancel();
            } catch (...) {
                qCritical(lcProjectionManager) << "Error cancelling USB workers.";
            }

            if (activeSession_ != nullptr) {
                try {
                    activeSession_->stop();
                    activeSession_.reset();
                } catch (...) {
                    qCritical(lcProjectionManager) << "Error stopping session.";
                }
            }
        });
    }

    void ProjectionManager::pause() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            if(activeSession_) {
                qInfo(lcProjectionManager) << "Pausing session.";
                activeSession_->pause();
            }
        });
    }

    void ProjectionManager::resume() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            if (activeSession_) {
                qInfo(lcProjectionManager) << "Resuming session.";
                activeSession_->resume();
            }
        });
    }

    void ProjectionManager::onAndroidAutoQuit() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcProjectionManager) << "Android Auto Quit requested.";

            if (activeSession_ != nullptr) {
                try {
                    activeSession_->stop();
                    activeSession_.reset();
                } catch (...) {
                    qCritical(lcProjectionManager) << "Error stopping session on quit.";
                }
            }

            if (!isStopped_) {
                try {
                    this->waitForDevice();
                } catch (...) {
                    qCritical(lcProjectionManager) << "Error restarting waitForDevice loop.";
                }
            }
        });
    }

    void ProjectionManager::enumerateDevices() {
        auto promise = aasdk::usb::IConnectedAccessoriesEnumerator::Promise::defer(strand_);
        promise->then([self = this->shared_from_this()](auto result) {
                          qInfo(lcProjectionManager) << "Enumeration result: " << result;
                      },
                      [self = this->shared_from_this()](auto e) {
                          qCritical(lcProjectionManager) << "Enumeration failed: " << e.what();
                      });

        connectedAccessoriesEnumerator_->enumerate(std::move(promise));
    }

    void ProjectionManager::waitForDevice() {
        androidAutoMonitor_->onConnectionStateUpdate(
            f1x::openauto::common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED);
        androidAutoMonitor_->onConnectionMethodUpdate(
            f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::AA_INDETERMINATE);

        qInfo(lcProjectionManager) << "Waiting for USB connection...";

        auto promise = aasdk::usb::IUSBHub::Promise::defer(strand_);
        promise->then(std::bind(&ProjectionManager::aoapDeviceHandler, this->shared_from_this(), std::placeholders::_1),
                      std::bind(&ProjectionManager::onUSBHubError, this->shared_from_this(), std::placeholders::_1));

        usbHub_->start(std::move(promise));
        startServerSocket();
    }

    void ProjectionManager::aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle) {
        qInfo(lcProjectionManager) << "USB Device Connected.";

        if (activeSession_ != nullptr) {
            qWarning(lcProjectionManager) << "Session already active. Ignoring new device.";
            return;
        }

        try {
            if (!disableAutostartEntity) {
                qInfo(lcProjectionManager) << "Autostarting USB Session.";
                connectedAccessoriesEnumerator_->cancel();

                auto aoapDevice(aasdk::usb::AOAPDevice::create(usbWrapper_, ioService_, deviceHandle));
                activeSession_ = sessionFactory_.create(std::move(aoapDevice));
                activeSession_->start(*this);
            } else {
                qInfo(lcProjectionManager) << "Autostart disabled.";
            }
        } catch (const aasdk::error::Error &error) {
            qCritical(lcProjectionManager) << "Failed to create USB Session: " << error.what();
            activeSession_.reset();
            this->waitForDevice();
        }
    }

    void ProjectionManager::onUSBHubError(const aasdk::error::Error &error) {
        qCritical(lcProjectionManager) << "USB Hub Error: " << error.what();
    }

    void ProjectionManager::startServerSocket() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            qInfo(lcProjectionManager) << "Listening for WiFi connections (Port 5000)";
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
            acceptor_.async_accept(
                *socket,
                std::bind(&ProjectionManager::handleNewClient, this, socket, std::placeholders::_1)
            );
        });
    }

    void ProjectionManager::handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err) {
        if (!err) {
            qInfo(lcProjectionManager) << "New WiFi Client Connected.";
            start(std::move(socket));
        } else {
            qWarning(lcProjectionManager) << "WiFi Accept Error: " << err.message().c_str();
        }
        // Always re-arm the acceptor so future connections are accepted.
        if (!isStopped_) {
            startServerSocket();
        }
    }
}