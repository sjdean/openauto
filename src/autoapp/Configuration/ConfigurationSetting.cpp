#include <QSettings>
#include <QString>
#include <QVariant>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>

namespace f1x::openauto::autoapp::configuration {

  void ConfigurationSetting::save(QSettings& settings) const {
    settings.setValue(m_name, QVariant::fromValue(m_value));
  }

  QString ConfigurationSetting::getName() const {
    return m_name;
  }
}
