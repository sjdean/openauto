#include <f1x/openauto/autoapp/Service/Sensor/SensorService.hpp>
#include <fstream>
#include <cmath>
#include <qloggingcategory.h>

#ifdef JOURNEYOS_CANBUS_RECEIVER
#include <aap_protobuf/service/sensorsource/message/SensorBatch.pb.h>
#include <aap_protobuf/service/sensorsource/message/SpeedData.pb.h>
#include <aap_protobuf/service/sensorsource/message/RpmData.pb.h>
#include <aap_protobuf/service/sensorsource/message/OdometerData.pb.h>
#include <aap_protobuf/service/sensorsource/message/EnvironmentData.pb.h>
#include <aap_protobuf/service/sensorsource/message/SensorType.pb.h>
#endif

Q_LOGGING_CATEGORY(lcServiceSensor, "journeyos.service.sensor")

namespace f1x::openauto::autoapp::service::sensor {
  SensorService::SensorService(aasdk::messenger::IMessenger::Pointer messenger)
      : channel_(std::make_shared<aasdk::channel::sensorsource::SensorSourceService>(std::move(messenger))) {
    timer_.setSingleShot(true);
    timer_.setInterval(250);
    QObject::connect(&timer_, &QTimer::timeout, [this]() { sensorPolling(); });
  }

  void SensorService::start() {
#ifdef Q_OS_LINUX
    if (is_file_exist("/tmp/night_mode_enabled"))
        this->isNight = true;
    this->sensorPolling();
#endif

#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (gpsReceiver_) {
        gpsConnection_ = QObject::connect(
            gpsReceiver_, &JourneyOS::GpsReceiver::fixUpdated,
            [this](double lat, double lon,
                   double altM,     bool hasAlt,
                   double speedMs,  bool hasSpeed,
                   double trackDeg, bool hasTrack,
                   double accuracyM) {
                sendGPSLocationData(lat, lon, altM, hasAlt, speedMs, hasSpeed, trackDeg, hasTrack, accuracyM);
            });
        qInfo(lcServiceSensor) << "GPS receiver connected (event-driven)";
    }
#endif

    qInfo(lcServiceSensor) << "starting";
    channel_->receive(this->shared_from_this());
  }

  void SensorService::stop() {
    this->stopPolling = true;
    timer_.stop();

#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (gpsConnection_)
        QObject::disconnect(gpsConnection_);
#endif
    qInfo(lcServiceSensor) << "stopping";
  }

  void SensorService::pause() {
      qDebug(lcServiceSensor) << "pausing";
  }

  void SensorService::resume() {
      qDebug(lcServiceSensor) << "resuming";
  }

  void SensorService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    qDebug(lcServiceSensor) << "filling features";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *sensorChannel = service->mutable_sensor_source_service();

    using ST = aap_protobuf::service::sensorsource::message::SensorType;
    sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_DRIVING_STATUS_DATA);
    sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_LOCATION);
    sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_NIGHT_MODE);

#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (canBusBridge_) {
        sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_SPEED);
        sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_RPM);
        sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_ODOMETER);
        sensorChannel->add_sensors()->set_sensor_type(ST::SENSOR_ENVIRONMENT_DATA);
        qInfo(lcServiceSensor) << "CAN bridge active: advertising speed/rpm/odometer/environment sensors";
    }
#endif
  }

  void SensorService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    qInfo(lcServiceSensor) << "channel open service_id=" << request.service_id() << " priority=" << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void SensorService::onSensorStartRequest(
      const aap_protobuf::service::sensorsource::message::SensorRequest &request) {
    qDebug(lcServiceSensor) << "sensor start request type=" << request.type();

    aap_protobuf::service::sensorsource::message::SensorStartResponseMessage response;
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

    auto promise = aasdk::channel::SendPromise::defer();

    if (request.type() == aap_protobuf::service::sensorsource::message::SENSOR_DRIVING_STATUS_DATA) {
      promise->then(std::bind(&SensorService::sendDrivingStatusUnrestricted, this->shared_from_this()),
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    } else if (request.type() == aap_protobuf::service::sensorsource::message::SensorType::SENSOR_NIGHT_MODE) {
      promise->then(std::bind(&SensorService::sendNightData, this->shared_from_this()),
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    } else {
      promise->then([]() {},
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    }

    channel_->sendSensorStartResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void SensorService::sendDrivingStatus(int flags) {
    qDebug(lcServiceSensor) << "sending driving status flags=" << flags;
    aap_protobuf::service::sensorsource::message::SensorBatch indication;
    indication.add_driving_status_data()->set_status(
        static_cast<aap_protobuf::service::sensorsource::message::DrivingStatus>(flags));

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
  }

  void SensorService::sendDrivingStatusUnrestricted() {
    sendDrivingStatus(
        static_cast<int>(aap_protobuf::service::sensorsource::message::DrivingStatus::DRIVE_STATUS_UNRESTRICTED));
  }

  void SensorService::sendNightData() {
    aap_protobuf::service::sensorsource::message::SensorBatch indication;
    qDebug(lcServiceSensor) << "night mode=" << SensorService::isNight;

    if (SensorService::isNight) {
      indication.add_night_mode_data()->set_night_mode(true);
    } else {
      indication.add_night_mode_data()->set_night_mode(false);
    }

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
    if (this->firstRun) {
      this->firstRun = false;
      this->previous = this->isNight;
    }
  }

  void SensorService::sendGPSLocationData(double lat, double lon,
                                           double altM,     bool hasAlt,
                                           double speedMs,  bool hasSpeed,
                                           double trackDeg, bool hasTrack,
                                           double accuracyM) {
    qDebug(lcServiceSensor) << "sending gps location lat=" << lat << "lon=" << lon;
    aap_protobuf::service::sensorsource::message::SensorBatch indication;

    auto* locInd = indication.add_location_data();
    locInd->set_latitude_e7(lat * 1e7);
    locInd->set_longitude_e7(lon * 1e7);
    locInd->set_accuracy_e3(accuracyM * 1e3);

    if (hasAlt)
        locInd->set_altitude_e2(altM * 1e2);
    if (hasSpeed)
        locInd->set_speed_e3(speedMs * 1.94384 * 1e3);  // m/s → knots × 1000
    if (hasTrack)
        locInd->set_bearing_e6(trackDeg * 1e6);

    auto promise = aasdk::channel::SendPromise::defer();
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
  }

  void SensorService::sensorPolling() {
    if (!this->stopPolling) {

#ifdef JOURNEYOS_CANBUS_RECEIVER
    if (canBusBridge_) {
        // Night mode from CAN dimmer/lights overrides the file-based check
        this->isNight = canBusBridge_->nightMode();

        // Push any changed CAN sensor values (speed, rpm, odometer, temperature)
        aap_protobuf::service::sensorsource::message::SensorBatch canBatch;
        if (canBusBridge_->fillSensorBatch(canBatch)) {
            auto promise = aasdk::channel::SendPromise::defer();
            promise->then([]() {},
                          std::bind(&SensorService::onChannelError,
                                    this->shared_from_this(), std::placeholders::_1));
            channel_->sendSensorEventIndication(canBatch, std::move(promise));
        }

        // Driving status: update when speed crosses the 5 mph threshold
        const int flags = canBusBridge_->drivingStatusFlags();
        if (flags != lastDrivingStatus_) {
            lastDrivingStatus_ = flags;
            sendDrivingStatus(flags);
        }
    } else {
        this->isNight = is_file_exist("/tmp/night_mode_enabled");
    }
#else
          this->isNight = is_file_exist("/tmp/night_mode_enabled");
#endif

    if (this->previous != this->isNight && !this->firstRun) {
      this->previous = this->isNight;
      this->sendNightData();
    }

    timer_.start();
  
    }
  }

  bool SensorService::is_file_exist(const char *fileName) {
    std::ifstream ifile(fileName, std::ios::in);
    return ifile.good();
  }

  void SensorService::onChannelError(const aasdk::error::Error &e) {
    qWarning(lcServiceSensor) << "channel error msg=" << e.what();
  }
}



