#ifndef OPENAUTO_RESOLUTIONMODELITEM_HPP
#define OPENAUTO_RESOLUTIONMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <service/media/sink/message/VideoCodecResolutionType.pb.h>

namespace f1x::openauto::autoapp::UI {
  class ResolutionModelItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(aap_protobuf::service::media::sink::message::VideoCodecResolutionType value READ getValue WRITE setValue)
  public:
    explicit ResolutionModelItem(QString display, aap_protobuf::service::media::sink::message::VideoCodecResolutionType value, QObject *parent = nullptr);

  protected: // Override the base class functions
    QString getDisplay() const;
    void setDisplay(const QString &display);
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType getValue() const;
    void setValue(const aap_protobuf::service::media::sink::message::VideoCodecResolutionType &value);

  private:
    QString m_display;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType m_value;
  };
}

#endif //OPENAUTO_RESOLUTIONMODELITEM_HPP