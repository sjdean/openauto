#include <f1x/openauto/autoapp/UI/LightHandler.hpp>
#include <utility>

namespace f1x::openauto::autoapp::UI {
  LightHandler::LightHandler(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration) :
      configuration_(std::move(configuration)) {

  }


}