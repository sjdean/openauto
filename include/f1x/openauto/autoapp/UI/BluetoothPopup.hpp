#ifndef OPENAUTO_BLUETOOTHPOPUP_H
#define OPENAUTO_BLUETOOTHPOPUP_H

#include <QtCore/QObject>

class BluetoothPopup : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString bluetoothName READ bluetoothName WRITE setBluetoothName NOTIFY bluetoothNameChanged)
  Q_PROPERTY(int bluetoothStatus READ bluetoothStatus WRITE setBluetoothStatus NOTIFY bluetoothStatusChanged)

public:
  explicit BluetoothPopup(QObject *parent = nullptr);

signals:
  void bluetoothNameChanged();
  void bluetoothStatusChanged();

private:
  QString bluetoothName();
  void setBluetoothName(QString value);

  int bluetoothStatus();
  void setBluetoothStatus(int value);
};

#endif //OPENAUTO_BLUETOOTHPOPUP_H