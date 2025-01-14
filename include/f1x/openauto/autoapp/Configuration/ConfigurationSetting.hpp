#ifndef OPENAUTO_CONFIGURATIONSETTING_HPP
#define OPENAUTO_CONFIGURATIONSETTING_HPP

#include <QSettings>
#include <QString>
#include <QVariant>

namespace f1x::openauto::autoapp::configuration {
  class ConfigurationSetting {
  public:

    template <typename T>
    ConfigurationSetting(const QString &name, const T &defaultValue)
        : m_name(name), m_hasValue(false), m_defaultValue(QVariant::fromValue(defaultValue)), m_value(QVariant::fromValue(defaultValue)) {

    }
    QString getName() const;

    void setValue(QVariant value) {
      m_value = value;
      m_hasValue = true;
    }

    template <typename T>
    T getValue() const {
      return m_hasValue ? m_value.value<T>() : m_defaultValue.value<T>();
    }

    QVariant getValue() const {
      return m_hasValue ? m_value : m_defaultValue;
    }


    void load(QSettings& settings) {
      setValue(settings.value(m_name, getValue()));
    }

    void save(QSettings& settings) const;

  private:
    QString m_name;
    bool m_hasValue;
    QVariant m_defaultValue;
    QVariant m_value;
  };
}
#endif//OPENAUTO_CONFIGURATIONSETTING_HPP