#ifndef OPENAUTO_PULSEAUDIOTDEVICEMODELITEM_HPP
#define OPENAUTO_PULSEAUDIOTDEVICEMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <service/media/sink/message/VideoCodecResolutionType.pb.h>

namespace f1x::openauto::autoapp::UI {
  class PulseAudioDeviceModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(QString value READ getValue WRITE setValue)
  public:
    explicit PulseAudioDeviceModelItem(QString display, QString value, QObject *parent = nullptr);

  protected: // Override the base class functions
    QString getDisplay() const;
    void setDisplay(const QString &display);
    QString getValue() const;
    void setValue(const QString &value);

  private:
    QString m_display;
    QString m_value;
  };
}

#endif //OPENAUTO_PULSEAUDIOTDEVICEMODELITEM_HPP