

#pragma once#include <boost/property_tree/ini_parser.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <iostream>
#include <string>
#include <fstream>#include <stdio.h>

namespace f1x::openauto::autoapp::configuration {

  class Configuration : public IConfiguration {
  public:
    Configuration();

    void load() override;
    void reset() override;
    void save() override;

    QString getCarMake() override;
    QString getCarModel() override;
    aap_protobuf::service::sensorsource::message::FuelType getFuelType() override;
    aap_protobuf::service::sensorsource::message::EvConnectorType getElectricChargingType() override;
    uint getBrightnessDayMin() override;
    uint getBrightnessDayMax() override;
    uint getBrightnessNightMin() override;
    uint getBrightnessNightMax() override;
    uint getBrightnessCurrent() override;
    uint getDPI() override;
    QRect getMargin() override;
    int getOMXLayer() override;
    bool getRotateDisplay() override;
    int getVideoType() override;
    uint getVolumePlaybackMin() override;
    uint getVolumePlaybackMax() override;
    uint getVolumePlaybackCurrent() override;
    uint getVolumeCaptureMin() override;
    uint getVolumeCaptureMax() override;
    uint getVolumeCaptureCurrent() override;
    AudioOutputBackendType getAudioType() override;
    bool getAutoStart() override;
    bool getAutoPlayback() override;
    bool getChannelMedia() override;
    bool getChannelGuidance() override;
    bool getChannelTelephony() override;
    aap_protobuf::service::media::sink::message::VideoFrameRateType getFrameRate() override;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() override;
    aap_protobuf::service::control::message::DriverPosition getDriverPosition() override;
  private:

    // Car Details
    QString carMake_;
    QString carModel_;
    aap_protobuf::service::sensorsource::message::FuelType fuelType_;
    aap_protobuf::service::sensorsource::message::EvConnectorType electricChargingType_;

    // Display
    uint brightnessDayMin_;
    uint brightnessDayMax_;
    uint brightnessNightMin_;
    uint brightnessNightMax_;
    uint brightnessCurrent_;
    uint dpi_;
    QRect margin_;
    int omxLayer_;
    bool rotateDisplay_;
    int videoType_;

    // Audio
    uint volumePlaybackMin_;
    uint volumePlaybackMax_;
    uint volumeCaptureMin_;
    uint volumeCaptureMax_;

    uint volumePlaybackCurrent_;
    uint volumeCaptureCurrent_;

    AudioOutputBackendType audioType_;

    // Media
    bool autoPlayback_;
    bool autoStart_;

    // AndroidAuto
    bool channelMedia_;
    bool channelGuidance_;
    bool channelTelephony_;
    aap_protobuf::service::media::sink::message::VideoFrameRateType videoFPS_;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType videoResolution_;

    // Bluetooth
    bool bluetoothEnabled_;
    QString bluetoothAdapterAddress_;

    // Wifi
    bool wifiEnabled_;
    QString wifiSsid_;
    QString wifiPassword_;

    aap_protobuf::service::control::message::DriverPosition driverPosition_;




  };

}



