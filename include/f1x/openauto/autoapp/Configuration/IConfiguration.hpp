#pragma once

#include <string>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationSetting.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationKeys.hpp>
#include <stdio.h>

#include <qloggingcategory.h>

namespace f1x::openauto::autoapp::configuration {
    using SettingType = std::variant<bool, int, QString>;

    class IConfiguration {
    public:
        typedef std::shared_ptr<IConfiguration> Pointer;
        typedef std::vector<aap_protobuf::service::media::sink::message::KeyCode> ButtonCodes;

        virtual ~IConfiguration() = default;

        // --- Enum overloads (preferred) --------------------------------------

        template<typename T>
        T getSettingByName(ConfigGroup group, ConfigKey key, T defaultValue = T{}) {
            return getSettingByName<T>(toQString(group), toQString(key), defaultValue);
        }

        template<typename T>
        void updateSettingByName(ConfigGroup group, ConfigKey key, T value) {
            updateSettingByName<T>(toQString(group), toQString(key), std::move(value));
        }

        // --- String overloads (internal / legacy) ----------------------------

        template<typename T>
        T getSettingByName(QString groupName, QString settingName, T defaultValue = T{}) {
            auto groupIt = std::find_if(m_configurationGroups.begin(), m_configurationGroups.end(),
                                        [&groupName](const ConfigurationGroup &group) {
                                            return group.getName() == groupName;
                                        });

            if (groupIt != m_configurationGroups.end()) {
                return (*groupIt).template getValueForSetting<T>(settingName);
            }
            qWarning() << "getSettingByName: group not found:" << groupName;
            return defaultValue;
        }

        template<typename T>
        void updateSettingByName(QString groupName, QString settingName, T value) {
            auto groupIt = std::find_if(m_configurationGroups.begin(), m_configurationGroups.end(),
                                        [&groupName](const ConfigurationGroup &group) {
                                            return group.getName() == groupName;
                                        });

            if (groupIt != m_configurationGroups.end()) {
                (*groupIt).setValueForSetting(settingName, value);
                onSettingChanged(groupName, settingName, QVariant::fromValue(value));
            } else {
                qWarning() << "updateSettingByName: group not found:" << groupName;
            }
        }

        // Called after every successful updateSettingByName so subclasses can
        // persist the individual key without rewriting the entire file.
        virtual void onSettingChanged(const QString& /*group*/, const QString& /*key*/, const QVariant& /*value*/) {}

        virtual bool hasTouchScreen() const = 0;

        virtual void save() const = 0;

        QList<ConfigurationGroup> m_configurationGroups;
    };
}
