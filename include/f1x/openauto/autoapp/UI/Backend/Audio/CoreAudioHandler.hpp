#pragma once
#if defined(__APPLE__)

#ifndef OPENAUTO_COREAUDIOHANDLER_HPP
#define OPENAUTO_COREAUDIOHANDLER_HPP

#include "IAudioHandler.h"
#include <QString>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Backend::Audio {

  class CoreAudioHandler : public IAudioHandler {

  public:
    explicit CoreAudioHandler();
    ~CoreAudioHandler() override;

    QString getDefaultSink() override;
    QString getDefaultSource() override;

    void setSinkMute(const QString& deviceName, bool mute) override;
    void setSinkVolume(const QString& deviceName, int volume) override;
    void setSourceMute(const QString& deviceName, bool mute) override;
    void setSourceVolume(const QString& deviceName, int volume) override;

    EngineDeviceList getSinks() override;
    EngineDeviceList getSources() override;
    std::vector<std::pair<std::string, std::string>> getDeviceList() override;
  };
}
#endif
#endif