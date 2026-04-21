#pragma once
#include <QMetaObject>
#include <QTimer>
#include <aap_protobuf/service/sensorsource/message/DrivingStatus.pb.h>
#include <aap_protobuf/service/sensorsource/message/SensorType.pb.h>
#include <aasdk/Channel/SensorSource/SensorSourceService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <aasdk/Messenger/IMessenger.hpp>

#ifdef JOURNEYOS_CANBUS_RECEIVER
#include <f1x/openauto/autoapp/Service/Sensor/CanBusSensorBridge.hpp>
#include <JourneyOS/CanBus/GpsReceiver.h>
#endif


namespace f1x::openauto::autoapp::service::sensor {
    class SensorService :
            public aasdk::channel::sensorsource::ISensorSourceServiceEventHandler,
            public IService,
            public std::enable_shared_from_this<SensorService> {
    public:
        SensorService(aasdk::messenger::IMessenger::Pointer messenger);

        bool isNight = false;
        bool previous = false;
        bool stopPolling = false;

#ifdef JOURNEYOS_CANBUS_RECEIVER
        // Call once after construction (before start()) to enable CAN sensor data.
        void setCanBusBridge(CanBusSensorBridge* bridge) { canBusBridge_ = bridge; }
        // Call once after construction (before start()) to enable event-driven GPS.
        void setGpsReceiver(JourneyOS::GpsReceiver* gps) { gpsReceiver_ = gps; }
#endif

        void start() override;

        void stop() override;

        void pause() override;

        void resume() override;

        void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

        void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

        void onSensorStartRequest(
            const aap_protobuf::service::sensorsource::message::SensorRequest &request) override;

        void onChannelError(const aasdk::error::Error &e) override;

    private:
        using std::enable_shared_from_this<SensorService>::shared_from_this;

        void sendDrivingStatusUnrestricted();
        void sendDrivingStatus(int flags);

        void sendNightData();

        void sendGPSLocationData(double lat, double lon,
                                  double altM,     bool hasAlt,
                                  double speedMs,  bool hasSpeed,
                                  double trackDeg, bool hasTrack,
                                  double accuracyM);

        bool is_file_exist(const char *filename);

        void sensorPolling();

        bool firstRun = true;

        QTimer timer_;
        aasdk::channel::sensorsource::SensorSourceService::Pointer channel_;

#ifdef JOURNEYOS_CANBUS_RECEIVER
        CanBusSensorBridge*     canBusBridge_      = nullptr;
        JourneyOS::GpsReceiver* gpsReceiver_       = nullptr;
        QMetaObject::Connection gpsConnection_;
        int                     lastDrivingStatus_ = -1;   // -1 = not yet sent
#endif
    };
}
