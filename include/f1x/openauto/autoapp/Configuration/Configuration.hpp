/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <boost/property_tree/ini_parser.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace configuration
{

class Configuration: public IConfiguration
{
public:
    Configuration();

    void load() override;
    void reset() override;
    void save() override;

    void setHandednessOfTrafficType(HandednessOfTrafficType value) override;
    HandednessOfTrafficType getHandednessOfTrafficType() const override;
    void showClock(bool value) override;
    bool showClock() const override;

    aasdk::proto::enums::VideoFPS::Enum getVideoFPS() const override;
    void setVideoFPS(aasdk::proto::enums::VideoFPS::Enum value) override;
    aasdk::proto::enums::VideoResolution::Enum getVideoResolution() const override;
    void setVideoResolution(aasdk::proto::enums::VideoResolution::Enum value) override;
    size_t getScreenDPI() const override;
    void setScreenDPI(size_t value) override;
    void setOMXLayerIndex(int32_t value) override;
    int32_t getOMXLayerIndex() const override;
    void setVideoMargins(QRect value) override;
    QRect getVideoMargins() const override;

    bool getTouchscreenEnabled() const override;
    void setTouchscreenEnabled(bool value) override;
    ButtonCodes getButtonCodes() const override;
    void setButtonCodes(const ButtonCodes& value) override;

    BluetoothAdapterType getBluetoothAdapterType() const override;
    void setBluetoothAdapterType(BluetoothAdapterType value) override;
    std::string getBluetoothRemoteAdapterAddress() const override;
    void setBluetoothRemoteAdapterAddress(const std::string& value) override;

    bool musicAudioChannelEnabled() const override;
    void setMusicAudioChannelEnabled(bool value) override;
    bool speechAudioChannelEnabled() const override;
    void setSpeechAudioChannelEnabled(bool value) override;
    AudioOutputBackendType getAudioOutputBackendType() const override;
    void setAudioOutputBackendType(AudioOutputBackendType value) override;

    // CubeOne Extensions
    void setCarRegistrationStatus(int32_t value) override;
    int32_t getCarRegistrationStatus() const override;
    void setCarRegistrationNumber(std::string value) override;
    std::string getCarRegistrationNumber() const override;
    void setCarModel(std::string value) override;
    std::string getCarModel() const override;
    void setCarMake(std::string value) override;
    std::string getCarMake() const override;
    void setCarKey(std::string value) override;
    std::string getCarKey() const override;
    void setAuthCode(std::string value) override;
    std::string getCarAuthCode() const override;
    void setDayBrightness(int32_t value) override;
    int32_t getDayBrightness() const override;
    void setNightBrightness(int32_t value) override;
    int32_t getNightBrightness() const override;

private:
    void readButtonCodes(boost::property_tree::ptree& iniConfig);
    void insertButtonCode(boost::property_tree::ptree& iniConfig, const std::string& buttonCodeKey, aasdk::proto::enums::ButtonCode::Enum buttonCode);
    void writeButtonCodes(boost::property_tree::ptree& iniConfig);

    HandednessOfTrafficType handednessOfTrafficType_;
    bool showClock_;
    aasdk::proto::enums::VideoFPS::Enum videoFPS_;
    aasdk::proto::enums::VideoResolution::Enum videoResolution_;
    size_t screenDPI_;
    int32_t omxLayerIndex_;
    QRect videoMargins_;
    bool enableTouchscreen_;
    ButtonCodes buttonCodes_;
    BluetoothAdapterType bluetoothAdapterType_;
    std::string bluetoothRemoteAdapterAddress_;
    bool musicAudioChannelEnabled_;
    bool speechAudiochannelEnabled_;
    AudioOutputBackendType audioOutputBackendType_;

    static const std::string cConfigFileName;

    static const std::string cGeneralShowClockKey;
    static const std::string cGeneralHandednessOfTrafficTypeKey;

    static const std::string cVideoFPSKey;
    static const std::string cVideoResolutionKey;
    static const std::string cVideoScreenDPIKey;
    static const std::string cVideoOMXLayerIndexKey;
    static const std::string cVideoMarginWidth;
    static const std::string cVideoMarginHeight;

    static const std::string cAudioMusicAudioChannelEnabled;
    static const std::string cAudioSpeechAudioChannelEnabled;
    static const std::string cAudioOutputBackendType;

    static const std::string cBluetoothAdapterTypeKey;
    static const std::string cBluetoothRemoteAdapterAddressKey;

    static const std::string cInputEnableTouchscreenKey;
    static const std::string cInputPlayButtonKey;
    static const std::string cInputPauseButtonKey;
    static const std::string cInputTogglePlayButtonKey;
    static const std::string cInputNextTrackButtonKey;
    static const std::string cInputPreviousTrackButtonKey;
    static const std::string cInputHomeButtonKey;
    static const std::string cInputPhoneButtonKey;
    static const std::string cInputCallEndButtonKey;
    static const std::string cInputVoiceCommandButtonKey;
    static const std::string cInputLeftButtonKey;
    static const std::string cInputRightButtonKey;
    static const std::string cInputUpButtonKey;
    static const std::string cInputDownButtonKey;
    static const std::string cInputScrollWheelButtonKey;
    static const std::string cInputBackButtonKey;
    static const std::string cInputEnterButtonKey;

    // CubeOne Customisations
    std::string carAuthCode_;
    std::string carKey_;
    int32_t carDayBrightness_;
    int32_t carNightBrightness_;
    int32_t carRegistrationStatus_;
    std::string carFuelType_;
    std::string carMake_;
    std::string carModel_;
    std::string carRegistrationNumber_;

    static const std::string cRegistrationStatus;
    static const std::string cDayBrightness;
    static const std::string cNightBrighenss;
    static const std::string cCarKey;
    static const std::string cAuthCode;
    static const std::string cFuelType;
    static const std::string cCarMake ;
    static const std::string cCarModel;
    static const std::string cCarRegistrationNumber;

};

}
}
}
}
