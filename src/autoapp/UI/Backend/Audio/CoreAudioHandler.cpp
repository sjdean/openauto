#if defined(__APPLE__)
#include <f1x/openauto/autoapp/UI/Backend/Audio/CoreAudioHandler.hpp>
#include <string>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcAudioCore, "journeyos.audio.core")

namespace f1x::openauto::autoapp::UI::Backend::Audio {
  /**
   *  Handles Pulse Audio through the IAudioHandler interface for Mac
   *  TODO: These are just stubs at the moment.
   */
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

  void CoreAudioHandler::setSinkVolume(const QString& deviceName, int volume) {

  }

  void CoreAudioHandler::setSourceVolume(const QString& deviceName, int volume) {

  }

  void CoreAudioHandler::setSinkMute(const QString& deviceName, bool mute) {

  }

  void CoreAudioHandler::setSourceMute(const QString& deviceName, bool mute) {

  }

  std::vector<std::pair<std::string, std::string>> CoreAudioHandler::getDeviceList() {
    std::vector<std::pair<std::string, std::string>> devices;

    return devices;
  }

  CoreAudioHandler::~CoreAudioHandler() {

  }
}
#endif