#ifndef OPENAUTO_CONFIGURATIONGROUP_HPP
#define OPENAUTO_CONFIGURATIONGROUP_HPP

#include <QSettings>
#include <QString>
#include "ConfigurationSetting.hpp"

namespace f1x::openauto::autoapp::configuration {
  using SettingType = std::variant<bool, int, QString>;

  class ConfigurationGroup {
  public:
    ConfigurationGroup(const QString &name, bool displayOnTab = true);

    template<typename T>
    void addSetting(const QString& name, const T& defaultValue) {
      auto configurationSetting = ConfigurationSetting(name, defaultValue);
      m_configurationSettings.emplace_back(name, configurationSetting);
    }


    template<typename T>
    T getValueForSetting(const QString& settingName) const {
      auto it = std::find_if(m_configurationSettings.begin(), m_configurationSettings.end(),
                             [&](const ConfigurationSetting& setting) {
                               return setting.getName() == settingName;
                             });

      if (it != m_configurationSettings.end()) {
        return it->template getValue<T>();
      } else {
        throw std::runtime_error("Setting not found: " + settingName.toStdString());
      }
    }

    template<typename T>
    void setValueForSetting(const QString& settingName, const T& value) {
      auto it = std::find_if(m_configurationSettings.begin(), m_configurationSettings.end(),
                             [&](const ConfigurationSetting& setting) {
                               return setting.getName() == settingName;
                             });

      if (it != m_configurationSettings.end()) {
        it->setValue(value);
      } else {
        throw std::runtime_error("Setting not found: " + settingName.toStdString());
      }
    }

    void load(QSettings &settings);
    void save(QSettings &settings) const;

    QString getName() const;

  private:
    QString m_name;
    bool m_displayOnTab;
    std::vector<ConfigurationSetting> m_configurationSettings;
  };
}
#endif//OPENAUTO_CONFIGURATIONGROUP_HPP