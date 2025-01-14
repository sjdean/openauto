#ifndef OPENAUTO_EVCONNECTORTYPEMODELITEM_HPP
#define OPENAUTO_EVCONNECTORTYPEMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <aap_protobuf/service/sensorsource/message/EvConnectorType.pb.h>

namespace f1x::openauto::autoapp::UI {
  class EvConnectorTypeModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(aap_protobuf::service::sensorsource::message::EvConnectorType value READ getValue WRITE setValue)
  public:
    explicit EvConnectorTypeModelItem(QString display, aap_protobuf::service::sensorsource::message::EvConnectorType value, QObject *parent = nullptr);

  protected: // Override the base class functions
    QString getDisplay() const;
    void setDisplay(const QString &display);
    aap_protobuf::service::sensorsource::message::EvConnectorType getValue() const;
    void setValue(const aap_protobuf::service::sensorsource::message::EvConnectorType &value);

  private:
    QString m_display;
    aap_protobuf::service::sensorsource::message::EvConnectorType m_value;
  };
}

#endif //OPENAUTO_EVCONNECTORTYPEMODELITEM_HPP