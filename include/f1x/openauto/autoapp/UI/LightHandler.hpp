#ifndef OPENAUTO_LIGHTHANDLER_HPP
#define OPENAUTO_LIGHTHANDLER_HPP

#include <QtCore/QObject>
#include <pulse/pulseaudio.h>
#include <cstdio>
#include <string>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/UI/PulseAudioHandler.hpp>

namespace f1x::openauto::autoapp::UI {
  class LightHandler {

  public:
    LightHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration);

    bool getIsDay();

  private:
    bool isDay;

  };

}

#endif//OPENAUTO_LIGHTHANDLER_HPP