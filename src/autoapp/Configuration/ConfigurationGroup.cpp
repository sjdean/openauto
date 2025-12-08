#include <QString>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcConfigGroup, "journeyos.configuration.group")

namespace f1x::openauto::autoapp::configuration {
  ConfigurationGroup::ConfigurationGroup(const QString& name, bool displayOnTab)
  : m_name(name), m_displayOnTab(displayOnTab)
  {
    m_configurationSettings.clear();
  }

  void ConfigurationGroup::load(QSettings& settings) {
    settings.beginGroup(m_name); // Use the group's name
    for (auto& setting : m_configurationSettings) {
      setting.load(settings); // Use QVariant for generic loading
    }
    settings.endGroup();
  }

  void ConfigurationGroup::save(QSettings& settings) const {
    fprintf(stderr, "Saving Grouo... %s\n", getName().toStdString().c_str());
    settings.beginGroup(m_name); // Use the group's name
    for (const auto& setting : m_configurationSettings) {
      fprintf(stderr, "Saving Setting %s\n", setting.getName().toStdString().c_str());
      setting.save(settings);
    }
    settings.endGroup();
  }

  QString ConfigurationGroup::getName() const {
    return m_name;
  }
}

