#pragma once

#include <string>
#include <QRect>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <f1x/openauto/autoapp/Configuration/BluetoothAdapterType.hpp>
#include <f1x/openauto/autoapp/Configuration/HandednessOfTrafficType.hpp>
#include <f1x/openauto/autoapp/Configuration/AudioOutputBackendType.hpp>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>
#include <f1x/openauto/autoapp/Configuration/ConfigurationGroup.hpp>
#include <f1x/openauto/autoapp/Configuration/ConfigurationSetting.hpp>
#include <stdio.h>

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
                                  [&groupName](const ConfigurationGroup& group) {
                                    return group.getName() == groupName;
                                  });

      if (groupIt != m_configurationGroups.end()) {
        return (*groupIt).template getValueForSetting<T>(settingName);
      } else {
        // Group not found
        fprintf(stderr, "Unable to find group 1 %s and setting %s\n", groupName.toStdString().c_str(), settingName.toStdString().c_str());
        //throw std::runtime_error("Group not found: " + groupName.toStdString());
      }
    }

    template<typename T>
    void updateSettingByName(QString groupName, QString settingName, T value) {
      auto groupIt = std::find_if(m_configurationGroups.begin(), m_configurationGroups.end(),
                                  [&groupName](const ConfigurationGroup& group) {
                                    return group.getName() == groupName;
                                  });

      if (groupIt != m_configurationGroups.end()) {
        (*groupIt).setValueForSetting(settingName, value);
        // TODO: Update Setting
      } else {
        // Group not found
        fprintf(stderr, "Unable to find group 2 %s\n", groupName.toStdString().c_str());
        //throw std::runtime_error("Group not found: " + groupName.toStdString());
      }
    }

    virtual bool hasTouchScreen() const = 0;

    QList<ConfigurationGroup> m_configurationGroups;
  };
}



