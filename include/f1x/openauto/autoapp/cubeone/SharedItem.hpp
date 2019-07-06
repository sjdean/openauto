#include"TelemetryItem.hpp"
namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace cubeone
            {
                public struct SharedItem {
                    char *carId;
                    TelemetryItem snapshot;
                    TelemetryItem *queue;
                    TelemetryItem *process;
                    int inJourney;
                    int telemetryOffset;
                    int startOffset;
                    int queueLength;
                    int processLength;
                    char *journeyid;
                    int dayBrightness;
                    int nightBrightness;
                }
            }
        }
    }
}
