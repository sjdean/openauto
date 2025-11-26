#ifndef OPENAUTO_LIGHTHANDLER_HPP
#define OPENAUTO_LIGHTHANDLER_HPP

#include <QObject>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {
  class LightHandler final : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool day READ getDay WRITE setDay NOTIFY dayChanged)
    Q_PROPERTY(bool lightsOn READ getLightsOn WRITE setLightsOn NOTIFY lightsOnChanged)

  public:
    explicit LightHandler(configuration::IConfiguration::Pointer configuration);

    void setDay(bool day);
    void setLightsOn(bool lightsOn);

    bool getDay() const;
    bool getLightsOn() const;

    signals:
    void dayChanged();
    void lightsOnChanged();

  private:
    configuration::IConfiguration::Pointer configuration_;
    bool m_isDay;
    bool m_isLightsOn;

  };

}

#endif//OPENAUTO_LIGHTHANDLER_HPP