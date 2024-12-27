#include <f1x/openauto/autoapp/UI/ComboBoxItem.hpp>

ComboBoxItem::ComboBoxItem(QObject *parent) :
    QObject(parent),
    m_value(0)
{

}

QString ComboBoxItem::display() const { return m_display; }
void ComboBoxItem::setDisplay(const QString &display) {
  if (m_display != display) {
    m_display = display;
    emit displayChanged();
  }
}

int ComboBoxItem::value() const { return m_value; }
void ComboBoxItem::setValue(int value) {
  if (m_value != value) {
    m_value = value;
    emit valueChanged();
  }
}
