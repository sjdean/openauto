#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>

namespace f1x::openauto::autoapp::configuration {

  void ConfigurationSetting::save(QSettings& settings) const {
    fprintf(stderr, "Saving Value %s\n", getValue().toString().toStdString().c_str());
    settings.setValue(m_name, getValue());
  }

  QString ConfigurationSetting::getName() const {
    return m_name;
  }
}
