#ifndef OPENAUTO_AUDIODEVICEMODELITEM_HPP // Renamed
#define OPENAUTO_AUDIODEVICEMODELITEM_HPP // Renamed

#include <QObject>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Model::List {
    // Renamed class
    class AudioDeviceModelItem : public QObject {
        Q_OBJECT
          Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
          Q_PROPERTY(QString value READ getValue WRITE setValue)
        public:
        explicit AudioDeviceModelItem(QString display, QString value, QObject *parent = nullptr); // Renamed

        QString getDisplay() const;
        void setDisplay(const QString &display);
        QString getValue() const;
        void setValue(const QString &value);

    private:
        QString m_display;
        QString m_value;
    };
}

#endif //OPENAUTO_AUDIODEVICEMODELITEM_HPP