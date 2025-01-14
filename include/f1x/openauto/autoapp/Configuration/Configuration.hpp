#pragma once
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include "ConfigurationGroup.hpp"

namespace f1x::openauto::autoapp::configuration {

  class Configuration : public IConfiguration {
  public:
    Configuration();

    bool hasTouchScreen() const override;

  private:

    QString generateRandomString(size_t length);

  };

}



