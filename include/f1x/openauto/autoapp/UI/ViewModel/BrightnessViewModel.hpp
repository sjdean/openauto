#ifndef OPENAUTO_BRIGHTNESSHANDLER_HPP
#define OPENAUTO_BRIGHTNESSHANDLER_HPP

#include <pulse/pulseaudio.h>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
  class BrightnessViewModel : public QObject {
  Q_OBJECT

    Q_PROPERTY(int brightness READ getBrightness WRITE setBrightness NOTIFY brightnessChanged)
  public:
    BrightnessViewModel(configuration::IConfiguration::Pointer configuration, Controller::LightController& lightHandler, QObject *parent = nullptr);

    void setBrightness(int userBrightnessTarget);
    int getBrightness() const;

  signals:
    void brightnessChanged();

  public slots:
    void onLightChange();

  private:
    static int calculateBrightness(int min, int max, int target);

    configuration::IConfiguration::Pointer m_configuration;
    Controller::LightController& m_lightHandler;

    int m_userBrightnessTarget{};
    int m_calculatedBrightness{};
  };

}

#endif//OPENAUTO_BRIGHTNESSHANDLER_HPP