

#pragma once#include <string>
#include <QRect>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <f1x/openauto/autoapp/Configuration/BluetoothAdapterType.hpp>
#include <f1x/openauto/autoapp/Configuration/HandednessOfTrafficType.hpp>
#include <f1x/openauto/autoapp/Configuration/AudioOutputBackendType.hpp>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>




      namespace f1x::openauto::autoapp::configuration {

        class IConfiguration {
        public:
          typedef std::shared_ptr<IConfiguration> Pointer;
          typedef std::vector<aap_protobuf::service::media::sink::message::KeyCode> ButtonCodes;

          virtual ~IConfiguration() = default;

          virtual void load() = 0;
          virtual void reset() = 0;
          virtual void save() = 0;

          virtual QString getCarMake() = 0;
          virtual QString getCarModel() = 0;
          virtual aap_protobuf::service::sensorsource::message::FuelType getFuelType() = 0;
          virtual aap_protobuf::service::sensorsource::message::EvConnectorType getElectricChargingType() = 0;
          virtual uint getBrightnessDayMin() = 0;
          virtual uint getBrightnessDayMax() = 0;
          virtual uint getBrightnessNightMin() = 0;
          virtual uint getBrightnessNightMax() = 0;
          virtual uint getBrightnessCurrent() = 0;
          virtual uint getDPI() = 0;
          virtual QRect getMargin() = 0;
          virtual int getOMXLayer() = 0;
          virtual bool getRotateDisplay() = 0;
          virtual int getVideoType() = 0;
          virtual uint getVolumePlaybackMin() = 0;
          virtual uint getVolumePlaybackMax() = 0;
          virtual  uint getVolumePlaybackCurrent() = 0;
          virtual  uint getVolumeCaptureMin() = 0;
          virtual uint getVolumeCaptureMax() = 0;
          virtual uint getVolumeCaptureCurrent() = 0;
          virtual AudioOutputBackendType getAudioType() = 0;
          virtual  bool getAutoStart() = 0;
          virtual bool getAutoPlayback() = 0;
          virtual bool getChannelMedia() = 0;
          virtual bool getChannelGuidance() = 0;
          virtual bool getChannelTelephony() = 0;
          virtual aap_protobuf::service::media::sink::message::VideoFrameRateType getFrameRate() = 0;
          virtual aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() = 0;
          virtual aap_protobuf::service::control::message::DriverPosition getDriverPosition() = 0;
        };
      }



