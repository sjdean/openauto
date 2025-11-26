#ifndef OPENAUTO_BRIGHTNESSHANDLER_HPP
#define OPENAUTO_BRIGHTNESSHANDLER_HPP

#include <pulse/pulseaudio.h>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <f1x/openauto/autoapp/UI/Monitor/LightHandler.hpp>

namespace f1x::openauto::autoapp::UI::Monitor {
  class BrightnessHandler : public QObject {
  Q_OBJECT

    Q_PROPERTY(int brightness READ getBrightness WRITE setBrightness NOTIFY brightnessChanged)
  public:
    BrightnessHandler(configuration::IConfiguration::Pointer configuration, LightHandler& lightHandler, QObject *parent = nullptr);

    void setBrightness(int userBrightnessTarget);
    int getBrightness() const;

  signals:
    void brightnessChanged();

  public slots:
    void onLightChange();

  private:
    static int calculateBrightness(int min, int max, int target);

    configuration::IConfiguration::Pointer configuration_;
    LightHandler& m_lightHandler;

    int m_userBrightnessTarget{};
    int m_calculatedBrightness{};

  };

}

#endif//OPENAUTO_BRIGHTNESSHANDLER_HPP