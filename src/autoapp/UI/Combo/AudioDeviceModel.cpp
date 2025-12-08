#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceModel.hpp" // Renamed

#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceDirection.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboAudio, "journeyos.audio.device.list")
namespace f1x::openauto::autoapp::UI::Combo {

  // Updated constructor
  AudioDeviceModel::AudioDeviceModel(std::shared_ptr<Monitor::IAudioHandler> audioHandler,
                                     AudioDeviceDirection direction,
                                     QObject *parent)
      : QObject(parent),
        m_currentComboBoxItem(nullptr),
        m_direction(direction),
        m_audioHandler(std::move(audioHandler)) { // Store the interface
      populateComboBoxItems();
  }

  void AudioDeviceModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    addComboBoxItem("None", "No Device Selected"); // Default

    // Use the generic interface and enum
    if (m_direction == AudioDeviceDirection::Input) {
      auto devices = m_audioHandler->getSources();
      for (auto &device : devices) {
        addComboBoxItem(device.description, device.value);
      }
    } else { // Output
      auto devices = m_audioHandler->getSinks();
      for (auto &device : devices) {
        addComboBoxItem(device.description, device.value);
      }
    }
  }

  QList<QObject *> AudioDeviceModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (AudioDeviceModelItem *item: m_comboBoxItems) { // Use renamed item
      list.append(item);
    }
    return list;
  }

  AudioDeviceModelItem* AudioDeviceModel::getCurrentComboBoxItem() { // Use renamed item
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void AudioDeviceModel::setCurrentComboBoxItem(AudioDeviceModelItem* value) { // Use renamed item
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

void AudioDeviceModel::addComboBoxItem(const QString &display, const QString& value) {
  auto item = new AudioDeviceModelItem(display, value, this);
  m_comboBoxItems.emplace_back(item);
  emit comboBoxItemsChanged();
}
}
