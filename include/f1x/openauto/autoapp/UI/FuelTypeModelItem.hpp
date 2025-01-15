#ifndef OPENAUTO_FUELTYPEMODELITEM_HPP
#define OPENAUTO_FUELTYPEMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <aap_protobuf/service/sensorsource/message/FuelType.pb.h>

namespace f1x::openauto::autoapp::UI {
  class FuelTypeModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(aap_protobuf::service::sensorsource::message::FuelType value READ getValue WRITE setValue)
  public:
    explicit FuelTypeModelItem(QString display, aap_protobuf::service::sensorsource::message::FuelType value, QObject *parent = nullptr);

    QString getDisplay() const;
    void setDisplay(const QString &display);
    aap_protobuf::service::sensorsource::message::FuelType getValue() const;
    void setValue(const aap_protobuf::service::sensorsource::message::FuelType &value);

  private:
    QString m_display;
    aap_protobuf::service::sensorsource::message::FuelType m_value;
  };
}

#endif //OPENAUTO_FUELTYPEMODELITEM_HPP