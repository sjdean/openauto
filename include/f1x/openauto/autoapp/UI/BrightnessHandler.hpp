#ifndef OPENAUTO_BRIGHTNESSHANDLER_HPP
#define OPENAUTO_BRIGHTNESSHANDLER_HPP

#include <QtCore/QObject>
#include <pulse/pulseaudio.h>
#include <cstdio>
#include <string>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

// TODO Need to bring in Light Handler to interpret between day and night

namespace f1x::openauto::autoapp::UI {
  class BrightnessHandler : public QObject {
  Q_OBJECT

    Q_PROPERTY(int brightness READ getBrightness WRITE setBrightness NOTIFY brightnessChanged)
  public:
    BrightnessHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, QObject *parent = nullptr);

  signals:
    void brightnessChanged();


  private:
    f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;

    void setBrightness(int brightness);
    int getBrightness();

    int m_brightness{};
    int calculateBrightness(int min, int max, int target);


  };

}

#endif//OPENAUTO_BRIGHTNESSHANDLER_HPP