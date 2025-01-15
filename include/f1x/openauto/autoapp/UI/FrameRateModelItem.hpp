#ifndef OPENAUTO_FRAMERATEMODELITEM_HPP
#define OPENAUTO_FRAMERATEMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>

namespace f1x::openauto::autoapp::UI {
  class FrameRateModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(aap_protobuf::service::media::sink::message::VideoFrameRateType value READ getValue WRITE setValue)
  public:
    explicit FrameRateModelItem(QString display, aap_protobuf::service::media::sink::message::VideoFrameRateType value, QObject *parent = nullptr);

    QString getDisplay() const;
    void setDisplay(const QString &display);
    aap_protobuf::service::media::sink::message::VideoFrameRateType getValue() const;
    void setValue(const aap_protobuf::service::media::sink::message::VideoFrameRateType &value);

  private:
    QString m_display;
    aap_protobuf::service::media::sink::message::VideoFrameRateType m_value;
  };
}

#endif //OPENAUTO_FRAMERATEMODELITEM_HPP