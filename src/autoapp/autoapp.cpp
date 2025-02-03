#include <thread>
#include <QScreen>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml/qqmlextensionplugin.h>

#include <pulse/pulseaudio.h>

#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChain.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>

#include <f1x/openauto/autoapp/App.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Configuration/RecentAddressesList.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoEntityFactory.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>
#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/Common/Log.hpp>

#include <f1x/openauto/autoapp/UI/SettingsView.hpp>

#include <f1x/openauto/autoapp/UI/FrameRateModel.hpp>
#include <f1x/openauto/autoapp/UI/ResolutionModel.hpp>

#include <f1x/openauto/autoapp/UI/DriverPositionModel.hpp>
#include <f1x/openauto/autoapp/UI/FuelTypeModel.hpp>
#include <f1x/openauto/autoapp/UI/EvConnectorTypeModel.hpp>

#include <f1x/openauto/autoapp/UI/PulseAudioDeviceModel.hpp>
#include <f1x/openauto/autoapp/UI/BluetoothAdapterModel.hpp>
#include <f1x/openauto/autoapp/UI/BrightnessHandler.hpp>
#include <f1x/openauto/autoapp/UI/VolumeHandler.hpp>

#include <f1x/openauto/autoapp/UI/AndroidAutoMonitor.hpp>
#include <f1x/openauto/autoapp/UI/WifiMonitor.hpp>
#include <f1x/openauto/autoapp/UI/PulseAudioHandler.hpp>
#include <f1x/openauto/autoapp/UI/BluetoothPopup.hpp>

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
  f1x::openauto::autoapp::UI::PulseAudioHandler pulseAudioHandler;


  set_qt_environment();

  auto configuration = std::make_shared<autoapp::configuration::Configuration>();

  // GUI
  QGuiApplication qApplication(argc, argv);
  QQmlApplicationEngine engine;
  QQuickView oj;

  qmlRegisterType<f1x::openauto::autoapp::UI::AndroidAutoConnectivityState>("AndroidAutoMonitor",1,0,"AndroidAutoConnectivityState");
  qmlRegisterType<f1x::openauto::autoapp::UI::AndroidAutoConnectivityMethod>("AndroidAutoMonitor",1,0,"AndroidAutoConnectivityMethod");
  qRegisterMetaType<f1x::openauto::autoapp::UI::AndroidAutoConnectivityState::Value>("AndroidAutoConnectivityState::Value");
  qRegisterMetaType<f1x::openauto::autoapp::UI::AndroidAutoConnectivityMethod::Value>("AndroidAutoConnectivityMethod::Value");
  qRegisterMetaType<f1x::openauto::autoapp::UI::BluetoothConnectionStatus::Value>("BluetoothConnectionStatus::Value");
  qRegisterMetaType<f1x::openauto::autoapp::UI::WirelessType::Value>("WirelessType::Value");

  // Initialise ComboBox Items - Backed by AA Enum's
  f1x::openauto::autoapp::UI::FrameRateModel frameRateModel;
  f1x::openauto::autoapp::UI::ResolutionModel resolutionModel;
  f1x::openauto::autoapp::UI::DriverPositionModel driverPositionModel;
  f1x::openauto::autoapp::UI::FuelTypeModel fuelTypeModel;
  f1x::openauto::autoapp::UI::EvConnectorTypeModel evConnectorTypeModel;

  engine.rootContext()->setContextProperty("frameRateModel", &frameRateModel);
  engine.rootContext()->setContextProperty("resolutionModel", &resolutionModel);
  engine.rootContext()->setContextProperty("driverPositionModel", &driverPositionModel);
  engine.rootContext()->setContextProperty("fuelTypeModel", &fuelTypeModel);
  engine.rootContext()->setContextProperty("evConnectorTypeModel", &evConnectorTypeModel);

  // Pulse Audio Input/Output (Sound) Devices
  f1x::openauto::autoapp::UI::PulseAudioDeviceModel pulseAudioDeviceModelOutput(pulseAudioHandler, pa_direction::PA_DIRECTION_OUTPUT);
  f1x::openauto::autoapp::UI::PulseAudioDeviceModel pulseAudioDeviceModelInput(pulseAudioHandler, pa_direction::PA_DIRECTION_INPUT);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelOutput", &pulseAudioDeviceModelOutput);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelInput", &pulseAudioDeviceModelInput);

  // Bluetooth Adapters on Host
  f1x::openauto::autoapp::UI::BluetoothAdapterModel bluetoothAdapterModel;
  //AvailableBluetoothDevicesForPairingModel availableBluetoothDevicesForPairingModel;

  engine.rootContext()->setContextProperty("bluetoothAdapterModel", &bluetoothAdapterModel);

  // Setting Handlers
  f1x::openauto::autoapp::UI::SettingsView settingsView(configuration);
  f1x::openauto::autoapp::UI::VolumeHandler volumeHandler(configuration, pulseAudioHandler);
  f1x::openauto::autoapp::UI::BrightnessHandler brightnessHandler(configuration);

  engine.rootContext()->setContextProperty("settingsViewHandler", &settingsView);
  engine.rootContext()->setContextProperty("volumePopupHandler", &volumeHandler);
  engine.rootContext()->setContextProperty("brightnessPopupHandler", &brightnessHandler);

  // Monitors
  auto androidAutoMonitor = std::make_shared<f1x::openauto::autoapp::UI::AndroidAutoMonitor>();
  auto wifiMonitor = std::make_shared<f1x::openauto::autoapp::UI::WifiMonitor>();
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

  autoapp::configuration::RecentAddressesList recentAddressesList(7);
  recentAddressesList.read();

  aasdk::tcp::TCPWrapper tcpWrapper;

  aasdk::usb::USBWrapper usbWrapper(usbContext);
  aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
  aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
  autoapp::service::ServiceFactory serviceFactory(ioService, configuration);
  autoapp::service::AndroidAutoEntityFactory androidAutoEntityFactory(ioService, configuration, serviceFactory, androidAutoMonitor);

  auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
  auto connectedAccessoriesEnumerator(
      std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));
  auto app = std::make_shared<autoapp::App>(ioService, usbWrapper, tcpWrapper, androidAutoEntityFactory,
                                            std::move(usbHub), std::move(connectedAccessoriesEnumerator), androidAutoMonitor);

  app->waitForUSBDevice();

  auto result = qApplication.exec();

  std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

  libusb_exit(usbContext);
  return result;
}