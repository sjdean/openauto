//
// Created by Simon Dean on 20/12/2024.
//#include <QtCore>
#include <f1x/openauto/autoapp/UI/FrameRateItem.hpp>

FrameRateItem::FrameRateItem(QObject *parent) :
    QObject(parent),
    m_value(0)
{

}


  QString FrameRateItem::display() const { return m_display; }
  void FrameRateItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;
      emit displayChanged();
    }
  }

  int FrameRateItem::value() const { return m_value; }
  void FrameRateItem::setValue(int value) {
    if (m_value != value) {
      m_value = value;
      emit valueChanged();
    }
  }
