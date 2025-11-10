#if defined(__APPLE__)
#include <f1x/openauto/autoapp/UI/Monitor/CoreAudioHandler.hpp>
#include <string>

namespace f1x::openauto::autoapp::UI::Monitor  {
  CoreAudioHandler::CoreAudioHandler() {

  }

  QString CoreAudioHandler::getDefaultSink() {
    QString defaultSinkName;



    return defaultSinkName;
  }


  EngineDeviceList CoreAudioHandler::getSinks() {
    return EngineDeviceList();

  }

  EngineDeviceList CoreAudioHandler::getSources() {

    return EngineDeviceList();

  }

  QString CoreAudioHandler::getDefaultSource() {
    QString defaultSourceName;

    return defaultSourceName;
  }

  void CoreAudioHandler::setSinkVolume(int volume) {

  }

  void CoreAudioHandler::setSourceVolume(int volume) {

  }

  void CoreAudioHandler::setSinkMute(bool mute) {

  }

  void CoreAudioHandler::setSourceMute(bool mute) {

  }

  std::vector<std::pair<std::string, std::string>> CoreAudioHandler::getDeviceList() {
    std::vector<std::pair<std::string, std::string>> devices;

    return devices;
  }

  CoreAudioHandler::~CoreAudioHandler() {

  }
}
#endif