#include <thread>
#include <QScreen>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml/qqmlextensionplugin.h>

#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>

#include <f1x/openauto/autoapp/ProjectionManager.hpp>
#include <f1x/openauto/autoapp/Service/SessionFactory.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>

// AASDK Backed Enum's
#include <f1x/openauto/autoapp/UI/Combo/DriverPositionModel.hpp>
#include <f1x/openauto/autoapp/UI/Combo/EvConnectorTypeModel.hpp>
#include <f1x/openauto/autoapp/UI/Combo/FrameRateModel.hpp>
#include <f1x/openauto/autoapp/UI/Combo/FuelTypeModel.hpp>
#include <f1x/openauto/autoapp/UI/Combo/ResolutionModel.hpp>

// Backed from System Calls
#include <f1x/openauto/autoapp/UI/Combo/BluetoothAdapterModel.hpp>
#include <f1x/openauto/autoapp/UI/Combo/BluetoothDeviceModel.hpp>

//
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

#include <f1x/openauto/autoapp/UI/Combo/NetworkAdapterModel.hpp>

#include <f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp>
#include "f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp"
#include "f1x/openauto/autoapp/UI/Controller/WifiController.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/WifiViewModel.hpp"

#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"
#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceModel.hpp"
#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/BrightnessViewModel.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/SettingsViewModel.hpp"

#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#if defined(__LINUX__)
#include <pulse/pulseaudio.h>
#include <f1x/openauto/autoapp/UI/Monitor/PulseAudioHandler.hpp>
#elif defined(__APPLE__)
#include "f1x/openauto/autoapp/UI/Monitor/CoreAudioHandler.hpp"
#endif

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcAutoapp, "journeyos")

namespace autoapp = f1x::openauto::autoapp;

using ThreadPool = std::vector<std::thread>;

Q_IMPORT_QML_PLUGIN(JourneyOSPlugin)
Q_IMPORT_QML_PLUGIN(JourneyOSContentPlugin)

const char mainQmlFile[] = "qrc:/qt/qml/JourneyOSContent/Journey.qml";

#ifdef BUILD_QDS_COMPONENTS
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_ComponentsPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_EffectsPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_ApplicationPlugin)
Q_IMPORT_QML_PLUGIN(FlowViewPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_LogicHelperPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_MultiTextPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_EventSimulatorPlugin)
Q_IMPORT_QML_PLUGIN(QtQuick_Studio_EventSystemPlugin)
#endif

inline void set_qt_environment() {
    qputenv("QML_COMPAT_RESOLVE_URLS_ON_ASSIGNMENT", "1");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    qputenv("QT_LOGGING_RULES", "qt.qml.connections=false");
    qputenv("QT_QUICK_CONTROLS_CONF", ":/qtquickcontrols2.conf");
}

void startUSBWorkers(boost::asio::io_service &ioService, libusb_context *usbContext, ThreadPool &threadPool) {
    auto usbWorker = [&ioService, usbContext]() {
        timeval libusbEventTimeout{5, 0};

        while (!ioService.stopped()) {
            libusb_handle_events_timeout_completed(usbContext, &libusbEventTimeout, nullptr);
        }
    };

    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
}

void startIOServiceWorkers(boost::asio::io_service &ioService, ThreadPool &threadPool) {
    auto ioServiceWorker = [&ioService]() {
        ioService.run();
    };

    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
    threadPool.emplace_back(ioServiceWorker);
}

int main(int argc, char *argv[]) {
    // Initial High Level Connectivity
    qInfo(lcAutoapp) << "OpenAuto booting up";
    libusb_context *usbContext;
    if (libusb_init(&usbContext) != 0) {
        qCritical(lcAutoapp) << "[AutoApp] libusb_init failed.";
        return 1;
    }

    // IO
    boost::asio::io_service ioService;
    boost::asio::io_service::work work(ioService);
    std::vector<std::thread> threadPool;

    // Start Workers
    qInfo(lcAutoapp) << "Starting Workers";
    startUSBWorkers(ioService, usbContext, threadPool);
    startIOServiceWorkers(ioService, threadPool);

    // Environment
    std::shared_ptr<autoapp::UI::Monitor::IAudioHandler> audioHandler;
#if defined(__LINUX__)
    audioHandler = std::make_shared<autoapp::UI::Monitor::PulseAudioHandler>();
#elif defined(__APPLE__)
    audioHandler = std::make_shared<autoapp::UI::Monitor::CoreAudioHandler>();
#endif

    qInfo(lcAutoapp) << "Reading Configuration";

    auto configuration = std::make_shared<autoapp::configuration::Configuration>();
    set_qt_environment();

    // GUI
    QGuiApplication app(argc, argv);

    // Only set pattern if the user hasn't overridden it externally
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        // LOGIC: Show function info if we are in DEBUG mode OR if the Context flag is enabled
#if !defined(QT_NO_DEBUG) || defined(QT_MESSAGELOGCONTEXT)
        // Function info is available (either Debug build or forced in Release)
        qSetMessagePattern("[%{time h:mm:ss}] [%{category}] [%{function}] %{message}");
#else
        // Release build AND context flag is missing.
        // Function info would read "[unknown]", so we remove it from the pattern.
        qSetMessagePattern("[%{time h:mm:ss}] [%{category}] %{message}");
#endif
    }

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    QQuickView oj;

    qInfo(lcAutoapp) << "Initialising GUI";

    // Type Registration - Use fully qualified names for safety with Signals/Slots
    qmlRegisterType<f1x::openauto::common::Enum::AndroidAutoConnectivityState>(
        "AndroidAutoMonitor", 1, 0, "AndroidAutoConnectivityState");
    qmlRegisterType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod>(
        "AndroidAutoMonitor", 1, 0, "AndroidAutoConnectivityMethod");
    qmlRegisterType<f1x::openauto::common::Enum::BluetoothConnectionStatus>(
        "AndroidAutoMonitor", 1, 0, "BluetoothConnectionStatus");
    qmlRegisterType<f1x::openauto::common::Enum::WirelessType>("AndroidAutoMonitor", 1, 0, "WirelessType");

    qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityState::Value>(
        "common::Enum::AndroidAutoConnectivityState::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::Value>(
        "f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::BluetoothConnectionStatus::Value>(
        "f1x::openauto::common::Enum::BluetoothConnectionStatus::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::WirelessType::Value>(
        "f1x::openauto::common::Enum::WirelessType::Value");

    // Initialise ComboBox Items - Backed by AA Enum's
    autoapp::UI::Combo::DriverPositionModel driverPositionModel;
    autoapp::UI::Combo::EvConnectorTypeModel evConnectorTypeModel;
    autoapp::UI::Combo::FrameRateModel frameRateModel;
    autoapp::UI::Combo::FuelTypeModel fuelTypeModel;
    autoapp::UI::Combo::ResolutionModel resolutionModel;

    context->setContextProperty("driverPositionModel", &driverPositionModel);
    context->setContextProperty("evConnectorTypeModel", &evConnectorTypeModel);
    context->setContextProperty("frameRateModel", &frameRateModel);
    context->setContextProperty("fuelTypeModel", &fuelTypeModel);
    context->setContextProperty("resolutionModel", &resolutionModel);

    // Pulse Audio Input/Output (Sound) Devices

    autoapp::UI::Combo::AudioDeviceModel pulseAudioDeviceModelOutput(audioHandler,
                                                                     autoapp::UI::Combo::AudioDeviceDirection::Output);
    autoapp::UI::Combo::AudioDeviceModel pulseAudioDeviceModelInput(audioHandler,
                                                                    autoapp::UI::Combo::AudioDeviceDirection::Input);
    context->setContextProperty("pulseAudioDeviceModelOutput", &pulseAudioDeviceModelOutput);
    context->setContextProperty("pulseAudioDeviceModelInput", &pulseAudioDeviceModelInput);

    auto wifiMonitor = new autoapp::UI::Monitor::WifiMonitor(configuration, &app);
    auto wifiController = new f1x::openauto::autoapp::UI::Controller::WifiController(configuration, &app);
    auto wifiViewModel = new f1x::openauto::autoapp::UI::ViewModel::WifiViewModel(configuration, wifiController, &app);

    autoapp::UI::Combo::NetworkAdapterModel networkAdapterModel;
    context->setContextProperty("networkAdapterModel", &networkAdapterModel);

    // Connect to Bluetooth
    autoapp::UI::Monitor::BluetoothHandler bluetoothHandler(configuration);

    // Bluetooth Adapters on Host
    autoapp::UI::Combo::BluetoothAdapterModel bluetoothAdapterModel;
    context->setContextProperty("bluetoothAdapterModel", &bluetoothAdapterModel);

    // Bluetooth Devices
    autoapp::UI::Combo::BluetoothDeviceModel bluetoothDeviceModel(&bluetoothHandler);
    context->setContextProperty("bluetoothDeviceModel", &bluetoothDeviceModel);

    context->setContextProperty("bluetoothHandler", &bluetoothHandler);
    context->setContextProperty("bluetoothViewModel", &bluetoothHandler);
    context->setContextProperty("bluetoothMonitor", &bluetoothHandler);
    context->setContextProperty("bluetoothPopupHandler", &bluetoothHandler);

    // Setting Handlers
    autoapp::UI::Controller::LightController lightHandler(configuration);
    autoapp::UI::Monitor::VolumeHandler volumeHandler(configuration, audioHandler);
    autoapp::UI::ViewModel::BrightnessViewModel brightnessHandler(configuration, lightHandler);

    autoapp::UI::ViewModel::SettingsViewModel settingsViewModel(configuration);

    context->setContextProperty("settingsViewHandler", &settingsViewModel);
    context->setContextProperty("volumePopupHandler", &volumeHandler);
    context->setContextProperty("brightnessPopupHandler", &brightnessHandler);
    engine.rootContext()->setContextProperty("wifiViewModel", wifiViewModel);

    // Monitors
    auto androidAutoMonitor = std::make_shared<autoapp::UI::Monitor::AndroidAutoMonitor>();
    context->setContextProperty("wifiMonitor", wifiMonitor);
    context->setContextProperty("androidAutoMonitor", androidAutoMonitor.get());

    // Bluetooth Status and Connectivity // DBus/BlueZ
    // Wifi Status // Other
    // Android Auto Status and Connectivity / Within App

    const QUrl url(mainQmlFile);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    // Screen Configuration
    int width = 0;
    int height = 0;

    qInfo(lcAutoapp) << "Reading Display Information";

    for (QScreen *screen: app.screens()) {
        qDebug(lcAutoapp) << "[AutoApp] Screen name: " << screen->name().toStdString();
        qDebug(lcAutoapp) << "[AutoApp] Screen geometry: "
                << screen->geometry().width(); // This includes position and size
        qDebug(lcAutoapp) << "[AutoApp] Screen physical size: " << screen->physicalSize().width(); // Size in millimeters
    }

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    // Check if a primary screen was found
    if (primaryScreen) {
        // Get the geometry of the primary screen
        QRect screenGeometry = primaryScreen->geometry();
        width = screenGeometry.width();
        height = screenGeometry.height();
        qInfo(lcAutoapp) << "Using gemoetry from primary screen.";
        qInfo(lcAutoapp) << "Display width: " << width;
        qInfo(lcAutoapp) << "Display height: " << height;
    } else {
        qInfo(lcAutoapp) << "Unable to find primary screen, using default values.";
    }

    aasdk::tcp::TCPWrapper tcpWrapper;
    aasdk::usb::USBWrapper usbWrapper(usbContext);
    aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
    aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
    autoapp::service::ServiceFactory serviceFactory(ioService, configuration);
    autoapp::service::SessionFactory sessionFactory(ioService, configuration, serviceFactory,
                                                                        androidAutoMonitor);

    auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
    auto connectedAccessoriesEnumerator(
        std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));

    qInfo(lcAutoapp) << "Starting up Projection Manager";
    auto oa = std::make_shared<autoapp::ProjectionManager>(configuration, ioService, usbWrapper, tcpWrapper, sessionFactory,
                                             std::move(usbHub), std::move(connectedAccessoriesEnumerator),
                                             androidAutoMonitor);

    qInfo(lcAutoapp) << "Waiting for USB";
    oa->waitForUSBDevice();

    // FIX: Stop the background services when the GUI shuts down
    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&ioService]() {
        qInfo(lcAutoapp) << "Stopping IO Service...";
        ioService.stop();
    });

    // Explicitly ensure the app quits when the last window closes
    app.setQuitOnLastWindowClosed(true);

    auto result = app.exec();

    std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

    libusb_exit(usbContext);
    return result;
}
