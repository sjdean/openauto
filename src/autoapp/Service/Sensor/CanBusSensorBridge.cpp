#ifdef JOURNEYOS_CANBUS_RECEIVER

#include <f1x/openauto/autoapp/Service/Sensor/CanBusSensorBridge.hpp>
#include <aap_protobuf/service/sensorsource/message/DrivingStatus.pb.h>
#include <aap_protobuf/service/sensorsource/message/SpeedData.pb.h>
#include <aap_protobuf/service/sensorsource/message/RpmData.pb.h>
#include <aap_protobuf/service/sensorsource/message/OdometerData.pb.h>
#include <aap_protobuf/service/sensorsource/message/EnvironmentData.pb.h>

namespace f1x::openauto::autoapp::service::sensor {

CanBusSensorBridge::CanBusSensorBridge(JourneyOS::CanBusReceiver* receiver)
    : receiver_(receiver) {}

bool CanBusSensorBridge::fillSensorBatch(
    aap_protobuf::service::sensorsource::message::SensorBatch& batch) {
    if (!receiver_) return false;
    bool added = false;

    // Speed: mph → m/s × 1000  (1 mph = 0.44704 m/s → × 447)
    const int speedMph = receiver_->speedMph();
    if (speedMph != lastSpeedMph_) {
        lastSpeedMph_ = speedMph;
        batch.add_speed_data()->set_speed_e3(speedMph * 447);
        added = true;
    }

    // RPM: value × 1000
    const int rpm = receiver_->rpm();
    if (rpm != lastRpm_) {
        lastRpm_ = rpm;
        batch.add_rpm_data()->set_rpm_e3(rpm * 1000);
        added = true;
    }

    // Odometer: miles → km × 10  (1 mile = 1.60934 km → × 16 ≈ ×1.6×10)
    const int odo = receiver_->odometerMiles();
    if (odo != lastOdometer_) {
        lastOdometer_ = odo;
        batch.add_odometer_data()->set_kms_e1(odo * 16);
        added = true;
    }

    // Outside temperature: °C × 1000
    const int tempC = receiver_->outsideTemp();
    if (tempC != lastTempC_) {
        lastTempC_ = tempC;
        batch.add_environment_data()->set_temperature_e3(tempC * 1000);
        added = true;
    }

    return added;
}

bool CanBusSensorBridge::nightMode() const {
    if (!receiver_) return false;
    // Night mode: dimmer wheel at ≤25% or sidelights/headlights on.
    // Design intent: day/night should eventually be time-based; lights state
    // is the best available proxy from CAN until time-of-day logic is added.
    return receiver_->dimmer() < 64 || receiver_->lights() >= 128;
}

int CanBusSensorBridge::drivingStatusFlags() const {
    if (!receiver_) return 0;
    using DS = aap_protobuf::service::sensorsource::message::DrivingStatus;
    if (receiver_->speedMph() > 5) {
        // Standard driving restrictions: no video, no keyboard, no voice input
        return DS::DRIVE_STATUS_NO_VIDEO
             | DS::DRIVE_STATUS_NO_KEYBOARD_INPUT
             | DS::DRIVE_STATUS_NO_VOICE_INPUT;
    }
    return DS::DRIVE_STATUS_UNRESTRICTED;
}

} // namespace f1x::openauto::autoapp::service::sensor

#endif // JOURNEYOS_CANBUS_RECEIVER