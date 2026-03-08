#ifndef OPENAUTO_BRIGHTNESSVIEWMODEL_HPP
#define OPENAUTO_BRIGHTNESSVIEWMODEL_HPP

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/UI/Controller/LightController.hpp"

namespace f1x::openauto::autoapp::UI::ViewModel {
  class BrightnessViewModel : public QObject {
  Q_OBJECT

    Q_PROPERTY(int targetBrightness READ getTargetBrightness WRITE setTargetBrightness NOTIFY targetBrightnessChanged)
    Q_PROPERTY(int screenBrightness READ getScreenBrightness NOTIFY screenBrightnessChanged)
    Q_PROPERTY(int currentMin READ getCurrentMin NOTIFY currentMinChanged)
    Q_PROPERTY(int currentMax READ getCurrentMax NOTIFY currentMaxChanged)

  public:
    BrightnessViewModel(configuration::IConfiguration::Pointer configuration, Controller::LightController& lightHandler, QObject *parent = nullptr);

    Q_INVOKABLE void saveSettings() const;

    void setTargetBrightness(int userBrightnessTarget);
    int getTargetBrightness() const;
    int getScreenBrightness() const;
    int getCurrentMin() const;
    int getCurrentMax() const;

signals:
    void targetBrightnessChanged();
    void screenBrightnessChanged();
    void currentMinChanged();
    void currentMaxChanged();

  public slots:
    void onLightChange();

  private:

    configuration::IConfiguration::Pointer m_configuration;
    Controller::LightController& m_lightHandler;

    int m_userBrightnessTarget{};
    int m_calculatedBrightness{};

#ifdef Q_OS_LINUX
    QString m_backlightPath;
    int m_backlightMaxBrightness{255};
    void applyHardwareBrightness(int calculatedBrightness);
#endif
  };

}

#endif//OPENAUTO_BRIGHTNESSVIEWMODEL_HPP