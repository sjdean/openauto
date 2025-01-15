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
    auto devices = PulseAudioDeviceModel::getPulseAudioDevices();
    for (auto &device : devices) {
      addComboBoxItem(QString::fromStdString(device.first), QString::fromStdString(device.second));
    }
  }

  std::vector<std::pair<std::string, std::string>> PulseAudioDeviceModel::getPulseAudioDevices() {
    return m_pulseAudioHandler.getDeviceList();
  }

  QList<QObject *> PulseAudioDeviceModel::getComboBoxItems() const {
    fprintf(stderr, "Getting Pulse Audio Devices\n");
    QList<QObject *> list;
    for (PulseAudioDeviceModelItem *item: m_comboBoxItems) {
      fprintf(stderr, "Appending to List...\n");
      list.append(item);
    }
    fprintf(stderr, "Returning to List...\n");
    return list;
  }

  PulseAudioDeviceModelItem* PulseAudioDeviceModel::getCurrentComboBoxItem() {
    fprintf(stderr, "Get Current ComboBox Item\n");
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      fprintf(stderr, "Empty or not set\n");
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    fprintf(stderr, "Returning Item name %s display %s\n", m_currentComboBoxItem->getValue().toStdString().c_str(), m_currentComboBoxItem->getDisplay().toStdString().c_str());
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