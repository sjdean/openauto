#include <f1x/openauto/autoapp/UI/PulseAudioDeviceModel.hpp>

namespace f1x::openauto::autoapp::UI {

  PulseAudioDeviceModel::PulseAudioDeviceModel(PulseAudioHandler pulseAudioHandler, pa_direction_t direction, QObject *parent)
      : QObject(parent),
        m_currentComboBoxItem(nullptr),
        m_direction(direction),
        m_pulseAudioHandler(pulseAudioHandler) {
      populateComboBoxItems();
  }

  void PulseAudioDeviceModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    addComboBoxItem("None", "No Device Selected");
    if (m_direction == pa_direction_t::PA_DIRECTION_INPUT) {
      auto devices = m_pulseAudioHandler.getSources();
      for (auto &device : devices) {
        addComboBoxItem(QString::fromStdString(device.first), QString::fromStdString(device.second));
      }
    } else {
      auto devices = m_pulseAudioHandler.getSinks();
      for (auto &device : devices) {
        addComboBoxItem(device.value.toString(), device.description);
      }
    }

    auto devices = PulseAudioDeviceModel::getPulseAudioDevices();

  }

  std::vector<std::pair<std::string, std::string>> PulseAudioDeviceModel::getPulseAudioDevices() {
    // TODO: Replace with get Sinks or getSources
    return m_pulseAudioHandler.getDeviceList();
  }

  QList<QObject *> PulseAudioDeviceModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (PulseAudioDeviceModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  PulseAudioDeviceModelItem* PulseAudioDeviceModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void PulseAudioDeviceModel::setCurrentComboBoxItem(PulseAudioDeviceModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void PulseAudioDeviceModel::addComboBoxItem(const QString &display, QString value) {
    auto item = new PulseAudioDeviceModelItem(display, value, this);
    m_comboBoxItems.emplace_back(item);
  }
}