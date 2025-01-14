#ifndef OPENAUTO_PULSEAUDIODEVICEMODEL_HPP
#define OPENAUTO_PULSEAUDIODEVICEMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <pulse/pulseaudio.h>
#include <iostream>
#include <vector>
#include <string>
#include <f1x/openauto/autoapp/UI/PulseAudioHandler.hpp>
#include <f1x/openauto/autoapp/UI/PulseAudioDeviceModelItem.hpp>

namespace f1x::openauto::autoapp::UI {
  class PulseAudioDeviceModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(PulseAudioDeviceModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit PulseAudioDeviceModel(PulseAudioHandler pulseAudioHandler, pa_direction_t direction, QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    PulseAudioDeviceModelItem* getCurrentComboBoxItem() const;
    void setCurrentComboBoxItem(PulseAudioDeviceModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, QString value);


  private:
    QList<PulseAudioDeviceModelItem *> m_comboBoxItems;
    PulseAudioDeviceModelItem* m_currentComboBoxItem;

    std::vector<std::pair<std::string, std::string>> getPulseAudioDevices();
    pa_direction_t m_direction;
    PulseAudioHandler m_pulseAudioHandler;
  };
}

#endif //OPENAUTO_PULSEAUDIODEVICEMODEL_HPP
