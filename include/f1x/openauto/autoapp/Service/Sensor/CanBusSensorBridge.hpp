#pragma once

#ifdef JOURNEYOS_CANBUS_RECEIVER

#include <JourneyOS/CanBus/CanBusReceiver.h>
#include <aap_protobuf/service/sensorsource/message/SensorBatch.pb.h>

namespace f1x::openauto::autoapp::service::sensor {

// Bridges live CAN bus data from JourneyOS::CanBusReceiver into AASDK sensor batches.
//
// Constructed once in main() and shared (by raw pointer) with each SensorService
// session instance. SensorService calls fillSensorBatch() in its 250 ms polling loop.
//
// The bridge compares each value against its last-sent cache and only adds entries
// that have changed, avoiding redundant data over the AA channel.
class CanBusSensorBridge {
public:
    explicit CanBusSensorBridge(JourneyOS::CanBusReceiver* receiver);

    // Populate a SensorBatch with any values that changed since last call.
    // Returns true if anything was added to the batch.
    bool fillSensorBatch(aap_protobuf::service::sensorsource::message::SensorBatch& batch);

    // Current night mode: dimmer wheel < 25% or sidelights/headlights on.
    bool nightMode() const;

    // Current driving status bitmask (DrivingStatus enum values OR-ed together).
    // 0 = UNRESTRICTED; 7 = NO_VIDEO|NO_KEYBOARD|NO_VOICE when speed > 5 mph.
    int drivingStatusFlags() const;

private:
    JourneyOS::CanBusReceiver* receiver_;

    // Last-sent cache — compare before adding to batch
    int lastSpeedMph_  = -1;
    int lastRpm_       = -1;
    int lastOdometer_  = -1;
    int lastTempC_     = -9999;
};

} // namespace f1x::openauto::autoapp::service::sensor

#endif // JOURNEYOS_CANBUS_RECEIVER