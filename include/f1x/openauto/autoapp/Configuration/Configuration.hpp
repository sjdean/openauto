#pragma once
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <fstream>

namespace f1x::openauto::autoapp::configuration {

  class Configuration : public IConfiguration {
  public:
    Configuration();

    bool hasTouchScreen() const override;
    void save() const override;

  private:
    static QString generateRandomString(size_t length);
    QSettings m_settings;
  };

}



