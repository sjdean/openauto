#ifndef OPENAUTO_COMBOBOXITEM_H
#define OPENAUTO_COMBOBOXITEM_H

#include <QObject>

class ComboBoxItem : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString display READ display WRITE setDisplay NOTIFY displayChanged)
  Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

public:
  explicit ComboBoxItem(QObject *parent = nullptr);

signals:

  void displayChanged();
  void valueChanged();

private:
  QString m_display;
  int m_value;

  QString display() const;
  void setDisplay(const QString &display);

  int value() const;
  void setValue(const int value);
};

#endif //OPENAUTO_COMBOBOXITEM_H
