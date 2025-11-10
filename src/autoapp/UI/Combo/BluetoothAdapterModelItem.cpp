#include "f1x/openauto/autoapp/UI/Combo/BluetoothAdapterModelItem.hpp"

namespace f1x::openauto::autoapp::UI::Combo {
  BluetoothAdapterModelItem::BluetoothAdapterModelItem(QString display, QString value, QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }
  
  QString BluetoothAdapterModelItem::getDisplay() const { return m_display; }
  
  void BluetoothAdapterModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  QString BluetoothAdapterModelItem::getValue() const {
    return m_value;
  }
  
  void BluetoothAdapterModelItem::setValue(const QString &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}