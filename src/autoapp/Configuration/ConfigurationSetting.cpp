#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcConfigSetting, "journeyos.configuration.setting")

namespace f1x::openauto::autoapp::configuration {

  void ConfigurationSetting::save(QSettings& settings) const {
    qDebug(lcConfigSetting) << "Saving Setting: " << getName() << " Value: " << getValue();
    settings.setValue(m_name, getValue());
  }

  QString ConfigurationSetting::getName() const {
    return m_name;
  }
}
