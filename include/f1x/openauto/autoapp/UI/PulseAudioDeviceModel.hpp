#ifndef OPENAUTO_PULSEAUDIODEVICEMODEL_H
#define OPENAUTO_PULSEAUDIODEVICEMODEL_H

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include "ComboBoxModel.hpp"
#include <pulse/pulseaudio.h>
#include <iostream>
#include <vector>
#include <string>

class PulseAudioDeviceModel : public ComboBoxModel {
  Q_OBJECT

public:
  explicit PulseAudioDeviceModel(pa_context* context, pa_direction_t direction, QObject *parent = nullptr);
protected:
  void populateComboBoxItems() override;
private:
  static std::vector<std::pair<std::string, std::string>> getPulseAudioDevices(pa_context* context, pa_direction_t direction);
  pa_context* m_context;
  pa_direction_t m_direction;
};

#endif //OPENAUTO_PULSEAUDIODEVICEMODEL_H
