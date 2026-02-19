#ifndef OPENAUTO_BRIGHTNESSVIEWMODEL_HPP
#define OPENAUTO_BRIGHTNESSVIEWMODEL_HPP

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
  class BrightnessViewModel : public QObject {
  Q_OBJECT

    Q_PROPERTY(int targetBrightness READ getTargetBrightness WRITE setTargetBrightness NOTIFY targetBrightnessChanged)
    Q_PROPERTY(int screenBrightness READ getScreenBrightness NOTIFY screenBrightnessChanged)

  public:
    BrightnessViewModel(configuration::IConfiguration::Pointer configuration, Controller::LightController& lightHandler, QObject *parent = nullptr);

    Q_INVOKABLE void saveSettings() const;

    void setTargetBrightness(int userBrightnessTarget);
    int getTargetBrightness() const;
    int getScreenBrightness() const;

signals:
    void targetBrightnessChanged();
    void screenBrightnessChanged();

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

#endif//OPENAUTO_BRIGHTNESSVIEWMODEL_HPP