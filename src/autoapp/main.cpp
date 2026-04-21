#include <thread>
#include <QFile>
#include <QScreen>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml/qqmlextensionplugin.h>

#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>

#include <f1x/openauto/autoapp/ProjectionManager.hpp>
#include <f1x/openauto/autoapp/Service/SessionFactory.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>

// AASDK Backed Enum's
#include <f1x/openauto/autoapp/UI/Model/List/DriverPositionModel.hpp>
#include <f1x/openauto/autoapp/UI/Model/List/EvConnectorTypeModel.hpp>
#include <f1x/openauto/autoapp/UI/Model/List/FrameRateModel.hpp>
#include <f1x/openauto/autoapp/UI/Model/List/FuelTypeModel.hpp>
#include <f1x/openauto/autoapp/UI/Model/List/ResolutionModel.hpp>

// Backed from System Calls
#include <f1x/openauto/autoapp/UI/Model/List/BluetoothAdapterModel.hpp>

//
#include <f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp>

#include <f1x/openauto/autoapp/UI/Model/List/NetworkAdapterModel.hpp>

#include "f1x/openauto/autoapp/UI/ViewModel/WifiViewModel.hpp"
#include "f1x/openauto/autoapp/UI/Manager/UpdateManager.hpp"
#include "f1x/openauto/autoapp/Hardware/HardwareProfile.hpp"

#ifdef Q_OS_LINUX
#include <f1x/openauto/autoapp/UI/Model/List/BluetoothDeviceModel.hpp>
#include <f1x/openauto/autoapp/UI/Monitor/LinuxBluetoothManager.hpp>
#include "f1x/openauto/autoapp/UI/Monitor/WifiMonitor.hpp"
#include "f1x/openauto/autoapp/UI/Controller/WifiController.hpp"
#else
#include "f1x/openauto/autoapp/UI/Monitor/NullBluetoothManager.hpp"
#include "f1x/openauto/autoapp/UI/Monitor/NullWiFiMonitor.hpp"
#include "f1x/openauto/autoapp/UI/Controller/NullWiFiController.hpp"
#endif

#include "f1x/openauto/autoapp/Configuration/Configuration.hpp"
#include "f1x/openauto/autoapp/Projection/IInputDevice.hpp"
#include "f1x/openauto/autoapp/Projection/InputDevice.hpp"
#include "f1x/openauto/autoapp/UI/Model/List/AudioDeviceModel.hpp"
#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"
#include "f1x/openauto/autoapp/UI/Controller/PowerController.hpp"
#include "f1x/openauto/autoapp/UI/Controller/TimeController.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/VolumeViewModel.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/BrightnessViewModel.hpp"
#include "f1x/openauto/autoapp/UI/ViewModel/SettingsViewModel.hpp"
#include <f1x/openauto/autoapp/UI/Model/List/CanBusDeviceModel.hpp>
#include <f1x/openauto/autoapp/Service/CanBusInputAdapter.hpp>

#ifdef JOURNEYOS_CANBUS_RECEIVER
#include <JourneyOS/CanBus/CanBusPortConfig.h>
#include <JourneyOS/CanBus/CanBusReceiver.h>
#include <JourneyOS/CanBus/DeviceManager.h>
#include <JourneyOS/CanBus/GpsReceiver.h>
#include <f1x/openauto/autoapp/Service/Sensor/CanBusSensorBridge.hpp>
#include <f1x/openauto/autoapp/Service/VinDecodeService.hpp>
#include <f1x/openauto/autoapp/Service/MappingLibraryService.hpp>
#endif

#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include "f1x/openauto/Common/Enum/BluetoothConnectionStatus.hpp"
#ifdef Q_OS_LINUX
#include <f1x/openauto/autoapp/UI/Backend/Audio/PipeWireAudioHandler.hpp>
#elif defined(__APPLE__)
#include "f1x/openauto/autoapp/UI/Backend/Audio/CoreAudioHandler.hpp"
#endif

#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcAutoapp, "journeyos")

namespace autoapp = f1x::openauto::autoapp;
using f1x::openauto::autoapp::configuration::ConfigGroup;
using f1x::openauto::autoapp::configuration::ConfigKey;
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
    qputenv("QT_IM_MODULE", "qtvirtualkeyboard");
    qputenv("QML_COMPAT_RESOLVE_URLS_ON_ASSIGNMENT", "1");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    // In release builds suppress verbose debug output; always silence QML connection noise
#if defined(QT_NO_DEBUG)
    qputenv("QT_LOGGING_RULES", "*.debug=false\nqt.qml.connections=false");
#else
    qputenv("QT_LOGGING_RULES", "qt.qml.connections=false");
#endif
    qputenv("QT_QUICK_CONTROLS_CONF", ":/qtquickcontrols2.conf");
}

    };

    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
    threadPool.emplace_back(usbWorker);
}


int main(int argc, char *argv[]) {
    // Initial High Level Connectivity
    qInfo(lcAutoapp) << "booting up";
    libusb_context *usbContext;
    if (libusb_init(&usbContext) != 0) {
        qCritical(lcAutoapp) << "libusb_init failed";
        return 1;
    }
    // Environment
    std::shared_ptr<autoapp::UI::Backend::Audio::IAudioHandler> audioHandler;
#ifdef Q_OS_LINUX
    audioHandler = std::make_shared<autoapp::UI::Backend::Audio::PipeWireAudioHandler>();
#elif defined(__APPLE__)
    audioHandler = std::make_shared<autoapp::UI::Backend::Audio::CoreAudioHandler>();
#else
    audioHandler = std::make_shared<autoapp::UI::Backend::Audio::NullAudioHandler>();
#endif

    qInfo(lcAutoapp) << "reading configuration";

    auto configuration = std::make_shared<autoapp::configuration::Configuration>();
    set_qt_environment();

    // GUI
    QApplication app(argc, argv);

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

    qInfo(lcAutoapp) << "initialising GUI";
    // HardwareProfile singleton — exposes hardware.json flags to QML
    // (written at boot by journeyos-hardware-detect; falls back to safe defaults if absent)
    qmlRegisterSingletonType<f1x::openauto::autoapp::Hardware::HardwareProfile>(
        "JourneyOS.Hardware", 1, 0, "HardwareProfile",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            auto* p = f1x::openauto::autoapp::Hardware::HardwareProfile::instance();
            QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
            return p;
        });

    qmlRegisterUncreatableType<f1x::openauto::common::Enum::AndroidAutoConnectivityState>(
            "AndroidAutoMonitor", 1, 0, "AndroidAutoConnectivityState", "Enum");

    qmlRegisterUncreatableType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod>(
        "AndroidAutoMonitor", 1, 0, "AndroidAutoConnectivityMethod", "Enum");

    qmlRegisterUncreatableType<f1x::openauto::common::Enum::BluetoothConnectionStatus::Value>(
        "AndroidAutoMonitor", 1, 0, "BluetoothConnectionStatus", "Enum");

    qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityState::Value>(
        "common::Enum::AndroidAutoConnectivityState::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::Value>(
        "common::Enum::AndroidAutoConnectivityMethod::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::BluetoothConnectionStatus::Value>(
        "f1x::openauto::common::Enum::BluetoothConnectionStatus::Value");
    qRegisterMetaType<f1x::openauto::common::Enum::WirelessType::Value>(
        "common::Enum::WirelessType::Value");

    // Initialise ComboBox Items - Backed by AA Enum's
    autoapp::UI::Model::List::DriverPositionModel driverPositionModel;
    autoapp::UI::Model::List::EvConnectorTypeModel evConnectorTypeModel;
    autoapp::UI::Model::List::FrameRateModel frameRateModel;
    autoapp::UI::Model::List::FuelTypeModel fuelTypeModel;
    autoapp::UI::Model::List::ResolutionModel resolutionModel;

#ifdef Q_OS_LINUX
    // Detect whether we are running on the JourneyOS Yocto image (head unit) or a
    // generic Linux desktop.  Read /etc/os-release and look for NAME=JourneyOS.
    // Any other Linux distribution is treated as a developer desktop (system managed).
    auto isJourneyOSPlatform = []() -> bool {
        QFile f(QStringLiteral("/etc/os-release"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;
        while (!f.atEnd()) {
            const QString line = QString::fromUtf8(f.readLine()).trimmed();
            if (line.startsWith(QLatin1String("NAME="), Qt::CaseInsensitive))
                return line.contains(QLatin1String("JourneyOS"), Qt::CaseInsensitive);
            if (line.startsWith(QLatin1String("ID="), Qt::CaseInsensitive))
                return line.contains(QLatin1String("journeyos"), Qt::CaseInsensitive);
        }
        return false;
    };
    bool isSystemManaged = !isJourneyOSPlatform(); // JourneyOS = head unit; any other Linux = desktop
#else
    bool isSystemManaged = true; // macOS / Windows — always desktop mode
#endif

    context->setContextProperty("isSystemManaged", isSystemManaged);
    context->setContextProperty("driverPositionModel", &driverPositionModel);
    context->setContextProperty("evConnectorTypeModel", &evConnectorTypeModel);
    context->setContextProperty("frameRateModel", &frameRateModel);
    context->setContextProperty("fuelTypeModel", &fuelTypeModel);
    context->setContextProperty("resolutionModel", &resolutionModel);

    // Pulse Audio Input/Output (Sound) Devices

    autoapp::UI::Model::List::AudioDeviceModel pulseAudioDeviceModelOutput(audioHandler,
                                                                     autoapp::UI::Model::List::AudioDeviceDirection::Output);
    autoapp::UI::Model::List::AudioDeviceModel pulseAudioDeviceModelInput(audioHandler,
                                                                    autoapp::UI::Model::List::AudioDeviceDirection::Input);
    context->setContextProperty("pulseAudioDeviceModelOutput", &pulseAudioDeviceModelOutput);
    context->setContextProperty("pulseAudioDeviceModelInput", &pulseAudioDeviceModelInput);

#ifdef Q_OS_LINUX
    auto* wifiMon = new autoapp::UI::Monitor::WifiMonitor(configuration, &app);
    auto* wifiCtrl = new autoapp::UI::Controller::WifiController(configuration, &app);
#else
    auto* wifiMon = new autoapp::UI::Monitor::NullWiFiMonitor(&app);
    auto* wifiCtrl = new autoapp::UI::Controller::NullWiFiController(&app);
#endif
    auto* wifiViewModel = new f1x::openauto::autoapp::UI::ViewModel::WifiViewModel(configuration, wifiCtrl, wifiMon, &app);

    autoapp::UI::Model::List::NetworkAdapterModel networkAdapterModel;
    context->setContextProperty("networkAdapterModel", &networkAdapterModel);

    // Connect to Bluetooth
    autoapp::UI::Monitor::IBluetoothManager* bluetoothManager = nullptr;
#ifdef Q_OS_LINUX
    auto* bluetoothHandlerConcrete = new autoapp::UI::Monitor::LinuxBluetoothManager(configuration, &app);
    bluetoothManager = bluetoothHandlerConcrete;
    auto* bluetoothDeviceModel = new autoapp::UI::Model::List::BluetoothDeviceModel(bluetoothHandlerConcrete, &app);
    context->setContextProperty("bluetoothDeviceModel", bluetoothDeviceModel);
#else
    bluetoothManager = new autoapp::UI::Monitor::NullBluetoothManager(&app);
    context->setContextProperty("bluetoothDeviceModel", nullptr);
#endif

    // Bluetooth Adapters on Host
    autoapp::UI::Model::List::BluetoothAdapterModel bluetoothAdapterModel;
    context->setContextProperty("bluetoothAdapterModel", &bluetoothAdapterModel);

    context->setContextProperty("bluetoothHandler", bluetoothManager);
    context->setContextProperty("bluetoothViewModel", bluetoothManager);
    context->setContextProperty("bluetoothMonitor", bluetoothManager);
    context->setContextProperty("bluetoothPopupHandler", bluetoothManager);

    // Setting Handlers
    autoapp::UI::Controller::LightController lightHandler(configuration);
    autoapp::UI::ViewModel::VolumeViewModel volumeHandler(configuration, audioHandler);
    autoapp::UI::ViewModel::BrightnessViewModel brightnessHandler(configuration, lightHandler);

    autoapp::UI::ViewModel::SettingsViewModel settingsViewModel(configuration);

    context->setContextProperty("settingsViewHandler", &settingsViewModel);
    context->setContextProperty("volumePopupHandler", &volumeHandler);
    context->setContextProperty("brightnessPopupHandler", &brightnessHandler);
    engine.rootContext()->setContextProperty("wifiViewModel", wifiViewModel);
    // Power Controller
    autoapp::System::PowerController powerController;
    context->setContextProperty("systemPower", &powerController);

    // Time Controller — synchronises system clock from NTP, RTC, or Android Auto
    auto* timeController = new autoapp::UI::Controller::TimeController(&app);
    context->setContextProperty("timeController", timeController);

    // Monitors
    auto androidAutoMonitor = std::make_shared<autoapp::UI::Monitor::AndroidAutoMonitor>();
    context->setContextProperty("wifiMonitor", wifiMon);
    context->setContextProperty("androidAutoMonitor", androidAutoMonitor.get());

    // Forward phone timestamp from AA session → TimeController (queued across thread boundary)
    QObject::connect(androidAutoMonitor.get(),
                     &autoapp::UI::Monitor::AndroidAutoMonitor::phoneTimestampReceived,
                     timeController,
                     &autoapp::UI::Controller::TimeController::offerTimeFromAndroidAuto);

    // OTA update manager (stub — no real HTTP yet)
    auto* updateManager = new f1x::openauto::autoapp::UI::UpdateManager(configuration, isSystemManaged, &app);
    context->setContextProperty("updateManager", updateManager);

    // ── CAN Bus device management ──────────────────────────────────────────────
    auto* canBusDeviceModel = new autoapp::UI::Model::List::CanBusDeviceModel(&app);
    context->setContextProperty("canBusDeviceModel", canBusDeviceModel);

#ifdef JOURNEYOS_CANBUS_RECEIVER
    f1x::openauto::autoapp::service::sensor::CanBusSensorBridge* canBusSensorBridge = nullptr;
    auto* gpsReceiver = new JourneyOS::GpsReceiver(&app);
    {
        // Pass the full mapping file path directly; DeviceManager falls back to its
        // installed template when the path is empty or the file doesn't exist.
        const QString mappingFile = configuration->getSettingByName<QString>(
            ConfigGroup::CanBus, ConfigKey::CanBusMappingFile);

        auto* canBusReceiver = new JourneyOS::CanBusReceiver(mappingFile, &app);
        context->setContextProperty("canBusReceiver", canBusReceiver);

        JourneyOS::CanBusPortConfig canPortConfig;   // loads ~/.config/journeyos/canbus_ports.json
        auto* deviceManager = new JourneyOS::DeviceManager(
            canPortConfig.portConfigs(),
            mappingFile,
            JourneyOS::DeviceManager::DEFAULT_PORT,
            &app);

        // DeviceManager emits from its worker thread — use QueuedConnection.
        QObject::connect(deviceManager, &JourneyOS::DeviceManager::deviceAnnounced,
                         canBusDeviceModel,
                         &autoapp::UI::Model::List::CanBusDeviceModel::onDeviceAnnounced,
                         Qt::QueuedConnection);
        QObject::connect(deviceManager, &JourneyOS::DeviceManager::deviceDisconnected,
                         canBusDeviceModel,
                         &autoapp::UI::Model::List::CanBusDeviceModel::onDeviceDisconnected,
                         Qt::QueuedConnection);

        // When a bus is confirmed ready, bind the receiver to that UDP port.
        // busReady is emitted from DeviceManager's worker thread — QueuedConnection
        // delivers it on canBusReceiver's worker thread (both use AutoConnection default).
        QObject::connect(deviceManager, &JourneyOS::DeviceManager::busReady,
                         canBusReceiver, &JourneyOS::CanBusReceiver::bindBus,
                         Qt::QueuedConnection);

        // Steering-wheel button events → volume control + AA key injection.
        auto* canBusInputAdapter = new f1x::openauto::autoapp::service::CanBusInputAdapter(
            volumeHandler, 10, &app);
        canBusInputAdapter->connectReceiver(canBusReceiver);

        // Pipe CAN telemetry into AASDK sensor channel (Stage 13).
        // Bridge is set on serviceFactory after it is constructed below.
        canBusSensorBridge =
            new f1x::openauto::autoapp::service::sensor::CanBusSensorBridge(canBusReceiver);

        qInfo(lcAutoapp) << "CAN bus device manager listening on port"
                         << JourneyOS::DeviceManager::DEFAULT_PORT;

        // VIN decode: bridge announces VIN → decode make/model/year → auto-populate Car tab.
        auto* vinDecodeService =
            new f1x::openauto::autoapp::service::VinDecodeService(configuration, &app);
        QObject::connect(deviceManager, &JourneyOS::DeviceManager::vinReceived,
                         vinDecodeService,
                         &f1x::openauto::autoapp::service::VinDecodeService::lookupVin,
                         Qt::QueuedConnection);
        QObject::connect(vinDecodeService,
                         &f1x::openauto::autoapp::service::VinDecodeService::decoded,
                         &settingsViewModel,
                         &autoapp::UI::ViewModel::SettingsViewModel::applyVinDecode,
                         Qt::QueuedConnection);

        // Community mapping library — exposes "mappingLibrary" to QML.
        auto* mappingLibrary =
            new f1x::openauto::autoapp::service::MappingLibraryService(&app);
        context->setContextProperty("mappingLibrary", mappingLibrary);

        // When the user downloads a map, auto-update the mapping file setting.
        QObject::connect(mappingLibrary,
                         &f1x::openauto::autoapp::service::MappingLibraryService::downloadComplete,
                         &settingsViewModel,
                         &autoapp::UI::ViewModel::SettingsViewModel::setCanBusMappingFile,
                         Qt::QueuedConnection);
    }
#else
    // Expose null placeholders so QML bindings don't crash on desktop builds.
    context->setContextProperty("canBusReceiver", QVariant());
    context->setContextProperty("mappingLibrary", QVariant());
    qInfo(lcAutoapp) << "CAN bus device management: disabled (library not found at build time)";
#endif

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

    qInfo(lcAutoapp) << "reading display information";

    for (QScreen *screen: app.screens()) {
        qDebug(lcAutoapp) << "screen name=" << screen->name()
                          << " geometry_w=" << screen->geometry().width()
                          << " physical_mm_w=" << screen->physicalSize().width();
    }

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    // Check if a primary screen was found
    if (primaryScreen) {
        // Get the geometry of the primary screen
        QRect screenGeometry = primaryScreen->geometry();
        width = screenGeometry.width();
        height = screenGeometry.height();
        qInfo(lcAutoapp) << "primary screen display_w=" << width << " display_h=" << height;
    } else {
        qWarning(lcAutoapp) << "no primary screen found, using defaults";
    }

    qInfo(lcAutoapp) << "initialising media infrastructure";

    autoapp::projection::IVideoOutput::Pointer videoOutput;
    videoOutput = std::make_shared<autoapp::projection::QtVideoOutput>(configuration);
    autoapp::projection::IAudioInput::Pointer audioInput;
    audioInput = std::make_shared<autoapp::projection::QtAudioInput>(1, 16, 16000, configuration);

    autoapp::projection::IAudioOutput::Pointer audioOutputMedia;
    autoapp::projection::IAudioOutput::Pointer audioOutputGuidance;
    autoapp::projection::IAudioOutput::Pointer audioOutputTelephony;
    autoapp::projection::IAudioOutput::Pointer audioOutputSystem;

    if (configuration->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoMedia)) {
        audioOutputMedia = std::make_shared<autoapp::projection::QtAudioOutput>(configuration);
    }

    if (configuration->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoGuidance)) {
        audioOutputGuidance = std::make_shared<autoapp::projection::QtAudioOutput>(configuration);
    }

    if (configuration->getSettingByName<bool>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoTelephony)) {
        audioOutputTelephony = std::make_shared<autoapp::projection::QtAudioOutput>(configuration);
    }
    audioOutputSystem = std::make_shared<autoapp::projection::QtAudioOutput>(configuration);

    // Requested Video Size
    QRect videoGeometry;
    switch (configuration->getSettingByName<int>(ConfigGroup::AndroidAuto, ConfigKey::AndroidAutoResolution)) {
        case aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1280x720:
            videoGeometry = QRect(0, 0, 1280, 720);
            break;
        case aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1920x1080:
            videoGeometry = QRect(0, 0, 1920, 1080);
            break;
        default:
            videoGeometry = QRect(0, 0, 800, 480);
            break;
    }

    // Get Screen Geometry
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen == nullptr ? QRect(0, 0, 800, 480) : screen->geometry();

    // Touchscreen geometry must match the actual window/view size so that QML
    // touch/mouse coordinates (which are in window space) map directly to the
    // coordinate range we declare to Android Auto.
    // - Head unit mode: window is fullscreen, so use the physical screen size.
    // - Windowed mode: window is fixed at 800x480 regardless of screen size.
    QRect touchscreenGeometry = settingsViewModel.isHeadUnitMode()
        ? screenGeometry
        : QRect(0, 0, 800, 480);

    // Create Input Device
    autoapp::projection::IInputDevice::Pointer inputDevice(
        std::make_shared<autoapp::projection::InputDevice>(*QApplication::instance(), configuration,
                                                           std::move(touchscreenGeometry), std::move(videoGeometry)));

       context->setContextProperty("inputMapper", dynamic_cast<QObject*>(inputDevice.get()));

    context->setContextProperty("videoBackend", dynamic_cast<QObject*>(videoOutput.get()));
    context->setContextProperty("audioBackendMedia", audioOutputMedia ? dynamic_cast<QObject*>(audioOutputMedia.get()) : nullptr);
    context->setContextProperty("audioBackendGuidance", audioOutputGuidance ? dynamic_cast<QObject*>(audioOutputGuidance.get()) : nullptr);
    context->setContextProperty("audioBackendTelephony", audioOutputTelephony ? dynamic_cast<QObject*>(audioOutputTelephony.get()) : nullptr);
    context->setContextProperty("audioBackendSystem", dynamic_cast<QObject*>(audioOutputSystem.get()));

    aasdk::usb::USBWrapper usbWrapper(usbContext);
    aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper);
    aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, queryFactory);
    autoapp::service::ServiceFactory serviceFactory(configuration, inputDevice, videoOutput, audioInput, audioOutputSystem, audioOutputMedia, audioOutputGuidance, audioOutputTelephony);
#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (canBusSensorBridge) {
        serviceFactory.setCanBusBridge(canBusSensorBridge);
        qInfo(lcAutoapp) << "CAN sensor bridge registered with ServiceFactory";
    }
    serviceFactory.setGpsReceiver(gpsReceiver);
    qInfo(lcAutoapp) << "GPS receiver registered with ServiceFactory (event-driven)";
#endif
    autoapp::service::SessionFactory sessionFactory(configuration, serviceFactory, androidAutoMonitor);

    auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, queryChainFactory));
    auto connectedAccessoriesEnumerator(
        std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, queryChainFactory));

    qInfo(lcAutoapp) << "starting projection manager";
    auto oa = std::make_shared<autoapp::ProjectionManager>(configuration, usbWrapper, sessionFactory,
                                             std::move(usbHub), std::move(connectedAccessoriesEnumerator),
                                             androidAutoMonitor);

    qInfo(lcAutoapp) << "waiting for USB device";
    oa->waitForUSBDevice();
    // Explicitly ensure the app quits when the last window closes
    app.setQuitOnLastWindowClosed(true);

    auto result = app.exec();
    libusb_exit(usbContext);
    return result;
}
