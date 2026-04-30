#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceModel.hpp"

#include "f1x/openauto/autoapp/UI/Combo/AudioDeviceDirection.hpp"
#include <qloggingcategory.h>
#include <QMetaObject>

Q_LOGGING_CATEGORY(lcComboAudio, "journeyos.audio.device.list")

namespace f1x::openauto::autoapp::UI::Combo {

  AudioDeviceModel::AudioDeviceModel(std::shared_ptr<Monitor::IAudioHandler> audioHandler,
                                     AudioDeviceDirection direction,
                                     QObject *parent)
      : QObject(parent),
        m_currentComboBoxItem(nullptr),
        m_direction(direction),
        m_audioHandler(std::move(audioHandler)) {
      populateComboBoxItems();

      if (m_direction == AudioDeviceDirection::Output) {
          m_audioHandler->addSinksChangedCallback([this]() {
              QMetaObject::invokeMethod(this, &AudioDeviceModel::refresh, Qt::QueuedConnection);
          });
      } else {
          m_audioHandler->addSourcesChangedCallback([this]() {
              QMetaObject::invokeMethod(this, &AudioDeviceModel::refresh, Qt::QueuedConnection);
          });
      }
  }

  void AudioDeviceModel::refresh() {
    populateComboBoxItems();
  }

  void AudioDeviceModel::populateComboBoxItems() {
    m_currentComboBoxItem = nullptr;
    qDeleteAll(m_comboBoxItems);
    m_comboBoxItems.clear();

    addComboBoxItem("None", "No Device Selected");

    if (m_direction == AudioDeviceDirection::Input) {
      auto devices = m_audioHandler->getSources();
      for (auto &device : devices) {
        addComboBoxItem(device.description, device.value);
      }
    } else {
      auto devices = m_audioHandler->getSinks();
      for (auto &device : devices) {
        addComboBoxItem(device.description, device.value);
      }
    }
    emit comboBoxItemsChanged();
  }

  QList<QObject *> AudioDeviceModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (AudioDeviceModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  AudioDeviceModelItem* AudioDeviceModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first();
    }
    return m_currentComboBoxItem;
  }

  void AudioDeviceModel::setCurrentComboBoxItem(AudioDeviceModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void AudioDeviceModel::addComboBoxItem(const QString &display, const QString& value) {
    auto item = new AudioDeviceModelItem(display, value, this);
    m_comboBoxItems.append(item);
  }
}
