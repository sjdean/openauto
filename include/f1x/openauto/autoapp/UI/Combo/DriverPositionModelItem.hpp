#ifndef OPENAUTO_DRIVERPOSITIONMODELITEM_HPP
#define OPENAUTO_DRIVERPOSITIONMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <aap_protobuf/service/control/message/DriverPosition.pb.h>

namespace f1x::openauto::autoapp::UI::Combo {
  class DriverPositionModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(aap_protobuf::service::control::message::DriverPosition value READ getValue WRITE setValue)
  public:
    explicit DriverPositionModelItem(QString display, aap_protobuf::service::control::message::DriverPosition value, QObject *parent = nullptr);

    QString getDisplay() const;
    void setDisplay(const QString &display);
    aap_protobuf::service::control::message::DriverPosition getValue() const;
    void setValue(const aap_protobuf::service::control::message::DriverPosition &value);

  private:
    QString m_display;
    aap_protobuf::service::control::message::DriverPosition m_value;
  };
}

#endif //OPENAUTO_DRIVERPOSITIONMODELITEM_HPP