//
// Created by Simon Dean on 20/12/2024.
//#include <QtCore>
#include <f1x/openauto/autoapp/UI/FuelTypeItem.hpp>

FuelTypeItem::FuelTypeItem(QObject *parent) :
    QObject(parent),
    m_value(0)
{

}


QString FuelTypeItem::display() const { return m_display; }
void FuelTypeItem::setDisplay(const QString &display) {
  if (m_display != display) {
    m_display = display;
    emit displayChanged();
  }
}

int FuelTypeItem::value() const { return m_value; }
void FuelTypeItem::setValue(int value) {
  if (m_value != value) {
    m_value = value;
    emit valueChanged();
  }
}
