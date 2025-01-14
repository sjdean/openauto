#pragma once
#include <gps.h>
#include <aap_protobuf/service/sensorsource/message/DrivingStatus.pb.h>
#include <aap_protobuf/service/sensorsource/message/SensorType.pb.h>
#include <aasdk/Channel/SensorSource/SensorSourceService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <boost/asio/io_service.hpp>
#include <aasdk/Messenger/IMessenger.hpp>


namespace f1x::openauto::autoapp::service::sensor {
  class SensorService :
      public aasdk::channel::sensorsource::ISensorSourceServiceEventHandler,
      public IService,
      public std::enable_shared_from_this<SensorService> {
  public:
    SensorService(boost::asio::io_service &ioService,
                  aasdk::messenger::IMessenger::Pointer messenger);

    bool isNight = false;
    bool previous = false;
    bool stopPolling = false;

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

    void sendNightData();

    void sendGPSLocationData();

    bool is_file_exist(const char *filename);

    void sensorPolling();

    bool firstRun = true;

    boost::asio::deadline_timer timer_;
    boost::asio::io_service::strand strand_;
    aasdk::channel::sensorsource::SensorSourceService::Pointer channel_;
    struct gps_data_t gpsData_;
    bool gpsEnabled_ = false;
  };

}



