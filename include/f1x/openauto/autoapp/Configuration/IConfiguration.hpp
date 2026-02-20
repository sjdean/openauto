#pragma once

#include <string>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationSetting.hpp>
#include <stdio.h>

#include <qloggingcategory.h>

namespace f1x::openauto::autoapp::configuration {
    using SettingType = std::variant<bool, int, QString>;

    class IConfiguration {
    public:
        typedef std::shared_ptr<IConfiguration> Pointer;
        typedef std::vector<aap_protobuf::service::media::sink::message::KeyCode> ButtonCodes;

        virtual ~IConfiguration() = default;

        template<typename T>
        T getSettingByName(QString groupName, QString settingName) {
            // Find the group
            auto groupIt = std::find_if(m_configurationGroups.begin(), m_configurationGroups.end(),
                                        [&groupName](const ConfigurationGroup &group) {
                                            return group.getName() == groupName;
                                        });

            if (groupIt != m_configurationGroups.end()) {
                return (*groupIt).template getValueForSetting<T>(settingName);
            } else {
                // Group not found
                qWarning() << "Unable to find requested group";
                //throw std::runtime_error("Group not found: " + groupName.toStdString());
            }
        }

        template<typename T>
        void updateSettingByName(QString groupName, QString settingName, T value) {
            qDebug() << "Updating group " << groupName << " setting " << settingName << " to " << value;
            auto groupIt = std::find_if(m_configurationGroups.begin(), m_configurationGroups.end(),
                                        [&groupName](const ConfigurationGroup &group) {
                                            return group.getName() == groupName;
                                        });

            if (groupIt != m_configurationGroups.end()) {
                qInfo() << "Setting Found";
                (*groupIt).setValueForSetting(settingName, value);
            } else {
                // Group not found
                qWarning() << "Unable to find requested group";
                //throw std::runtime_error("Group not found: " + groupName.toStdString());
            }
        }

        virtual bool hasTouchScreen() const = 0;

        virtual void save() const = 0;

        QList<ConfigurationGroup> m_configurationGroups;
    };
}
