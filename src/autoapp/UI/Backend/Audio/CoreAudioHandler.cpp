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

CoreAudioHandler::CoreAudioHandler() = default;
CoreAudioHandler::~CoreAudioHandler() = default;

QString CoreAudioHandler::getDefaultSink()   { return {}; }
QString CoreAudioHandler::getDefaultSource() { return {}; }

void CoreAudioHandler::setSinkVolume(const QString&, int volume) {
    qInfo(lcAudioCore) << "setSinkVolume" << volume << "(stub — macOS not yet implemented)";
}
void CoreAudioHandler::setSourceVolume(const QString&, int volume) {
    qInfo(lcAudioCore) << "setSourceVolume" << volume << "(stub)";
}
void CoreAudioHandler::setSinkMute(const QString&, bool mute) {
    qInfo(lcAudioCore) << "setSinkMute" << mute << "(stub)";
}
void CoreAudioHandler::setSourceMute(const QString&, bool mute) {
    qInfo(lcAudioCore) << "setSourceMute" << mute << "(stub)";
}

EngineDeviceList CoreAudioHandler::getSinks()   { return {}; }
EngineDeviceList CoreAudioHandler::getSources() { return {}; }

std::vector<std::pair<std::string, std::string>> CoreAudioHandler::getDeviceList() {
    return {};
}

} // namespace
#endif