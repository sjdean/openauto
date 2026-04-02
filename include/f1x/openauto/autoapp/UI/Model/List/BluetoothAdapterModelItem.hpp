#ifndef OPENAUTO_BLUETOOTHADAPTERMODELITEM_HPP
#define OPENAUTO_BLUETOOTHADAPTERMODELITEM_HPP

#include <QObject>
#include <QVariant>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>

namespace f1x::openauto::autoapp::UI::Model::List {
  class BluetoothAdapterModelItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(QString value READ getValue WRITE setValue)
  public:
    explicit BluetoothAdapterModelItem(QString display, QString value, QObject *parent = nullptr);

    QString getDisplay() const;
    void setDisplay(const QString &display);
    QString getValue() const;
    void setValue(const QString &value);

  private:
    QString m_display;
    QString m_value;
  };
}

#endif //OPENAUTO_BLUETOOTHADAPTERMODELITEM_HPP