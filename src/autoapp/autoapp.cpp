#include <thread>
#include <QScreen>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
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

pa_mainloop *m;
pa_mainloop_api *mainloop_api;
pa_context *context;

void initPulseAudio() {
  m = pa_mainloop_new();
  mainloop_api = pa_mainloop_get_api(m);

  context = pa_context_new(mainloop_api, "Your Application Name");
  pa_context_set_state_callback(context,
                                [](pa_context *c, void *userdata) {
                                  if (pa_context_get_state(c) == PA_CONTEXT_READY) {
                                    pa_threaded_mainloop_signal((pa_threaded_mainloop *) userdata, 0);
                                  }
                                }, m);

  if (pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
    fprintf(stderr, "Failed to connect to PulseAudio server\n");
    return;
  }

  pa_threaded_mainloop *loop = pa_threaded_mainloop_new();
  pa_threaded_mainloop_start(loop);

  // Wait for the context to be ready
  if (pa_context_get_state(context) != PA_CONTEXT_READY) {
    pa_threaded_mainloop_wait(loop);
  }

  pa_threaded_mainloop_free(loop);
}

int main(int argc, char *argv[]) {
  // Initialize PulseAudio here
  initPulseAudio();

  libusb_context *usbContext;
  if (libusb_init(&usbContext) != 0) {
    OPENAUTO_LOG(error) << "[AutoApp] libusb_init failed.";
    return 1;
  }

  boost::asio::io_service ioService;
  boost::asio::io_service::work work(ioService);
  std::vector<std::thread> threadPool;
  startUSBWorkers(ioService, usbContext, threadPool);
  startIOServiceWorkers(ioService, threadPool);

  set_qt_environment();
  QGuiApplication qApplication(argc, argv);
  QQmlApplicationEngine engine;
  QQuickView oj;

  SettingsView cppHandler;

  FrameRateModel frameRateModel;
  ResolutionModel resolutionModel;

  DriverPositionModel driverPositionModel;
  FuelTypeModel fuelTypeModel;
  EvConnectorTypeModel evConnectorTypeModel;

  PulseAudioDeviceModel pulseAudioDeviceModelOutput(context, pa_direction::PA_DIRECTION_OUTPUT);
  PulseAudioDeviceModel pulseAudioDeviceModelInput(context, pa_direction::PA_DIRECTION_INPUT);

  BluetoothAdapterModel bluetoothAdapterModel;
  //AvailableBluetoothDevicesForPairingModel availableBluetoothDevicesForPairingModel;

  engine.rootContext()->setContextProperty("cppHandler", &cppHandler);

  engine.rootContext()->setContextProperty("bluetoothAdapterModel", &bluetoothAdapterModel);
  engine.rootContext()->setContextProperty("driverPositionModel", &driverPositionModel);
  engine.rootContext()->setContextProperty("evConnectorTypeModel", &evConnectorTypeModel);
  engine.rootContext()->setContextProperty("frameRateModel", &frameRateModel);
  engine.rootContext()->setContextProperty("fuelTypeModel", &fuelTypeModel);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelOutput", &pulseAudioDeviceModelOutput);
  engine.rootContext()->setContextProperty("pulseAudioDeviceModelInput", &pulseAudioDeviceModelInput);

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

  auto configuration = std::make_shared<autoapp::configuration::Configuration>();

  autoapp::configuration::RecentAddressesList recentAddressesList(7);
  recentAddressesList.read();

  aasdk::tcp::TCPWrapper tcpWrapper;

  aasdk::usb::USBWrapper usbWrapper(usbContext);
  aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
  aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
  autoapp::service::ServiceFactory serviceFactory(ioService, configuration);
  autoapp::service::AndroidAutoEntityFactory androidAutoEntityFactory(ioService, configuration, serviceFactory);

  auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
  auto connectedAccessoriesEnumerator(
      std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));
  auto app = std::make_shared<autoapp::App>(ioService, usbWrapper, tcpWrapper, androidAutoEntityFactory,
                                            std::move(usbHub), std::move(connectedAccessoriesEnumerator));

  app->waitForUSBDevice();

  auto result = qApplication.exec();

  std::for_each(threadPool.begin(), threadPool.end(), std::bind(&std::thread::join, std::placeholders::_1));

  libusb_exit(usbContext);
  return result;
}