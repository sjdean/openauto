#include "f1x/openauto/autoapp/UI/Combo/BluetoothDeviceModelItem.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboBtDeviceItem, "journeyos.bluetooth.client.list.item")

namespace f1x::openauto::autoapp::UI::Combo {
  BluetoothDeviceModelItem::BluetoothDeviceModelItem(QString display, QString value, QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }

  QString BluetoothDeviceModelItem::getDisplay() const { return m_display; }

  void BluetoothDeviceModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  QString BluetoothDeviceModelItem::getValue() const {
    return m_value;
  }

  void BluetoothDeviceModelItem::setValue(const QString &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}
