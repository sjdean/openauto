#ifndef OPENAUTO_BLUETOOTHDEVICEMODELITEM_HPP
#define OPENAUTO_BLUETOOTHDEVICEMODELITEM_HPP

#include <QObject>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Model::List {
  class BluetoothDeviceModelItem : public QObject {
  Q_OBJECT
    Q_PROPERTY(QString display READ getDisplay WRITE setDisplay)
    Q_PROPERTY(QString value READ getValue WRITE setValue)
  public:
    explicit BluetoothDeviceModelItem(QString display, QString value, QObject *parent = nullptr);

    QString getDisplay() const;
    void setDisplay(const QString &display);
    QString getValue() const;
    void setValue(const QString &value);

  private:
    QString m_display;
    QString m_value;
  };
}

#endif //OPENAUTO_BLUETOOTHDEVICEMODELITEM_HPP