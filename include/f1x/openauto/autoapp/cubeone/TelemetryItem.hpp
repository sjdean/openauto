
#include"TelemetryItem.hpp"
namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace cubeone
            {
                public struct TelemetryItem
                {
                    int offset;
                    double longitude;
                    double latitude;
                    double altitude;
                    int gpsfix;
                    int gpsfirstfix;
                    timestamp_t gpsfixtime;
                    timestamp_t gpsfirstfixtime;
                    int speedmph;
                    int steeringposition;
                    int rpm;
                    int throttleposition;
                    int radiatortemperature;
                    int batteryvoltage;
                    int servicemiles;
                    int rangemiles;
                    int rangelitres;
                    int clutchstatus;
                    int brakestatus;
                    int brakeclutch;
                    int odometermiles;
                    int outsidetemperature;
                    int engine_status;
                    int lights;
                    int lights_high;
                    int lights_fog;
                    int dimmer;
                };
            }
        }
    }
}


