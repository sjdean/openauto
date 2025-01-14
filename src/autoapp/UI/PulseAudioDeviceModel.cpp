#include <f1x/openauto/autoapp/UI/PulseAudioDeviceModel.hpp>

namespace f1x::openauto::autoapp::UI {

  PulseAudioDeviceModel::PulseAudioDeviceModel(PulseAudioHandler pulseAudioHandler, pa_direction_t direction, QObject *parent)
      : QObject(parent),
        m_direction(std::move(direction)),
        m_pulseAudioHandler(std::move(pulseAudioHandler)) {
      populateComboBoxItems();
  }

  void PulseAudioDeviceModel::populateComboBoxItems() {
    auto devices = PulseAudioDeviceModel::getPulseAudioDevices();
    for (auto &device : devices) {
      addComboBoxItem(QString::fromStdString(device.first), QString::fromStdString(device.second));
    }
  }

  std::vector<std::pair<std::string, std::string>> PulseAudioDeviceModel::getPulseAudioDevices() {
    return m_pulseAudioHandler.getDeviceList();
  }

  QList<QObject *> PulseAudioDeviceModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (PulseAudioDeviceModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  PulseAudioDeviceModelItem* PulseAudioDeviceModel::getCurrentComboBoxItem() const { return m_currentComboBoxItem; }

  void PulseAudioDeviceModel::setCurrentComboBoxItem(PulseAudioDeviceModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void PulseAudioDeviceModel::addComboBoxItem(const QString &display, QString value) {
    auto item = PulseAudioDeviceModelItem(display, value);
    m_comboBoxItems.emplace_back(&item);
  }
}