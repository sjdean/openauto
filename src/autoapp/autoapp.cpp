#include <thread>
#include <QScreen>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml/qqmlextensionplugin.h>

#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChain.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>

#include <f1x/openauto/autoapp/App.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoEntityFactory.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>
#include <f1x/openauto/Common/Log.hpp>

#include <f1x/openauto/autoapp/UI/SettingsView.hpp>

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
#include <f1x/openauto/autoapp/UI/Monitor/BluetoothHandler.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/BrightnessHandler.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/LightHandler.hpp>

#include <f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp>

#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"
#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceModel.hpp"

#include "f1x/openauto/autoapp/UI/Monitor/VolumeHandler.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#if defined(__LINUX__)
#include <pulse/pulseaudio.h>
#include <f1x/openauto/autoapp/UI/Monitor/PulseAudioHandler.hpp>
#elif defined(__APPLE__)
#include "f1x/openauto/autoapp/UI/Monitor/CoreAudioHandler.hpp"
#endif



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
    timeval libusbEventTimeout{180, 0};

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
  libusb_context *usbContext;
  if (libusb_init(&usbContext) != 0) {
    OPENAUTO_LOG(error) << "[AutoApp] libusb_init failed.";
    return 1;
  }

  // IO
  boost::asio::io_service ioService;
  boost::asio::io_service::work work(ioService);
  std::vector<std::thread> threadPool;

  // Start Workers
  startUSBWorkers(ioService, usbContext, threadPool);
  startIOServiceWorkers(ioService, threadPool);

  // Environment
  std::shared_ptr<autoapp::UI::Monitor::IAudioHandler> audioHandler;
#if defined(__LINUX__)
  audioHandler = std::make_shared<autoapp::UI::Monitor::PulseAudioHandler>();
#elif defined(__APPLE__)
  audioHandler = std::make_shared<autoapp::UI::Monitor::CoreAudioHandler>();
#endif

  set_qt_environment();

  auto configuration = std::make_shared<autoapp::configuration::Configuration>();

  // GUI
  QGuiApplication qApplication(argc, argv);
  QQmlApplicationEngine engine;
  QQuickView oj;

  // Type Registration
  qmlRegisterType<f1x::openauto::common::Enum::AndroidAutoConnectivityState>("AndroidAutoMonitor",1,0,"AndroidAutoConnectivityState");
  qmlRegisterType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod>("AndroidAutoMonitor",1,0,"AndroidAutoConnectivityMethod");
  qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityState::Value>("AndroidAutoConnectivityState::Value");
  qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::Value>("AndroidAutoConnectivityMethod::Value");
  qRegisterMetaType<f1x::openauto::common::Enum::BluetoothConnectionStatus::Value>("BluetoothConnectionStatus::Value");
  qRegisterMetaType<f1x::openauto::common::Enum::WirelessType::Value>("WirelessType::Value");

  // Initialise ComboBox Items - Backed by AA Enum's
  autoapp::UI::Combo::DriverPositionModel driverPositionModel;
  autoapp::UI::Combo::EvConnectorTypeModel evConnectorTypeModel;
  autoapp::UI::Combo::FrameRateModel frameRateModel;
  autoapp::UI::Combo::FuelTypeModel fuelTypeModel;
  autoapp::UI::Combo::ResolutionModel resolutionModel;

  engine.rootContext()->setContextProperty("driverPositionModel", &driverPositionModel);
  engine.rootContext()->setContextProperty("evConnectorTypeModel", &evConnectorTypeModel);
  engine.rootContext()->setContextProperty("frameRateModel", &frameRateModel);
  engine.rootContext()->setContextProperty("fuelTypeModel", &fuelTypeModel);
  engine.rootContext()->setContextProperty("resolutionModel", &resolutionModel);

  // Pulse Audio Input/Output (Sound) Devices

  autoapp::UI::Combo::AudioDeviceModel pulseAudioDeviceModelOutput(audioHandler, autoapp::UI::Combo::AudioDeviceDirection::Output);
  autoapp::UI::Combo::AudioDeviceModel pulseAudioDeviceModelInput(audioHandler, autoapp::UI::Combo::AudioDeviceDirection::Input);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelOutput", &pulseAudioDeviceModelOutput);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelInput", &pulseAudioDeviceModelInput);

  // Connect to Bluetooth
  autoapp::UI::Monitor::BluetoothHandler bluetoothHandler(configuration);

  // Bluetooth Adapters on Host
  autoapp::UI::Combo::BluetoothAdapterModel bluetoothAdapterModel;
  engine.rootContext()->setContextProperty("bluetoothAdapterModel", &bluetoothAdapterModel);

  // Bluetooth Devices
  autoapp::UI::Combo::BluetoothDeviceModel bluetoothDeviceModel(&bluetoothHandler);
  engine.rootContext()->setContextProperty("bluetoothAdapterModel", &bluetoothDeviceModel);

  // Setting Handlers
  autoapp::UI::Monitor::LightHandler lightHandler(configuration);
  autoapp::UI::Monitor::VolumeHandler volumeHandler(configuration, audioHandler);

  autoapp::UI::Monitor::BrightnessHandler brightnessHandler(configuration, lightHandler);

  autoapp::UI::SettingsView settingsView(configuration);

  engine.rootContext()->setContextProperty("settingsViewHandler", &settingsView);

  engine.rootContext()->setContextProperty("volumePopupHandler", &volumeHandler);

  engine.rootContext()->setContextProperty("brightnessPopupHandler", &brightnessHandler);

  // Monitors
  auto androidAutoMonitor = std::make_shared<autoapp::UI::Monitor::AndroidAutoMonitor>();
  auto wifiMonitor = std::make_shared<autoapp::UI::Monitor::WifiMonitor>(configuration);
  engine.rootContext()->setContextProperty("androidAutoMonitor", androidAutoMonitor.get());
  engine.rootContext()->setContextProperty("wifiMonitor", wifiMonitor.get());

  // Bluetooth Status and Connectivity // DBus/BlueZ
  // Wifi Status // Other
  // Android Auto Status and Connectivity / Within App

  const QUrl url(mainQmlFile);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &qApplication,
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

  for (QScreen *screen: qApplication.screens()) {
    OPENAUTO_LOG(info) << "[AutoApp] Screen name: " << screen->name().toStdString();
    OPENAUTO_LOG(info) << "[AutoApp] Screen geometry: "
                       << screen->geometry().width(); // This includes position and size
    OPENAUTO_LOG(info) << "[AutoApp] Screen physical size: " << screen->physicalSize().width(); // Size in millimeters
  }

  QScreen *primaryScreen = QGuiApplication::primaryScreen();

  // Check if a primary screen was found
  if (primaryScreen) {
    // Get the geometry of the primary screen
    QRect screenGeometry = primaryScreen->geometry();
    width = screenGeometry.width();
    height = screenGeometry.height();
    OPENAUTO_LOG(info) << "[AutoApp] Using gemoetry from primary screen.";
  } else {
    OPENAUTO_LOG(info) << "[AutoApp] Unable to find primary screen, using default values.";
  }

  OPENAUTO_LOG(info) << "[AutoApp] Display width: " << width;
  OPENAUTO_LOG(info) << "[AutoApp] Display height: " << height;

  //f1x::openauto::configuration::RecentAddressesList recentAddressesList(7);
  //recentAddressesList.read();

  aasdk::tcp::TCPWrapper tcpWrapper;

  aasdk::usb::USBWrapper usbWrapper(usbContext);
  aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
  aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
  autoapp::service::ServiceFactory serviceFactory(ioService, configuration);
  autoapp::service::AndroidAutoEntityFactory androidAutoEntityFactory(ioService, configuration, serviceFactory, androidAutoMonitor);

  auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
  auto connectedAccessoriesEnumerator(
      std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));
  auto app = std::make_shared<autoapp::App>(configuration, ioService, usbWrapper, tcpWrapper, androidAutoEntityFactory,
                                            std::move(usbHub), std::move(connectedAccessoriesEnumerator), androidAutoMonitor);

  app->waitForUSBDevice();

  auto result = qApplication.exec();

  std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

  libusb_exit(usbContext);
  return result;
}