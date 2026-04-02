#ifndef OPENAUTO_AUDIODEVICEMODEL_HPP // Renamed
#define OPENAUTO_AUDIODEVICEMODEL_HPP // Renamed

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <iostream>
#include <vector>
#include <string>
#include <memory> // For std::shared_ptr

// Include the generic interface and new enum
#include <f1x/openauto/autoapp/UI/Model/List/AudioDeviceModelItem.hpp>

#include "AudioDeviceDirection.hpp"
#include "f1x/openauto/autoapp/UI/Backend/Audio/IAudioHandler.h"

namespace f1x::openauto::autoapp::UI::Model::List {
    // Renamed class
    class AudioDeviceModel : public QObject {
        Q_OBJECT
          Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
          Q_PROPERTY(AudioDeviceModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

        public:
        // Updated constructor to be platform-agnostic
        explicit AudioDeviceModel(std::shared_ptr<Backend::Audio::IAudioHandler> audioHandler,
                                  AudioDeviceDirection direction,
                                  QObject *parent = nullptr);

        signals:
          void comboBoxItemsChanged();
        void currentComboBoxItemChanged();

    protected:
        QList<QObject *> getComboBoxItems() const;
        AudioDeviceModelItem* getCurrentComboBoxItem();
        void setCurrentComboBoxItem(AudioDeviceModelItem* value);

        void populateComboBoxItems();
        void addComboBoxItem(const QString &display, const QString &value);

    private:
        QList<AudioDeviceModelItem *> m_comboBoxItems;
        AudioDeviceModelItem* m_currentComboBoxItem;

        // Store the generic enum and interface
        AudioDeviceDirection m_direction;
        std::shared_ptr<Backend::Audio::IAudioHandler> m_audioHandler;
    };
}

#endif //OPENAUTO_AUDIODEVICEMODEL_HPP