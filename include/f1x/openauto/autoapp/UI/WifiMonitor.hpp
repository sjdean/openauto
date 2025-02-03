#ifndef OPENAUTO_WIFIMONITOR_HPP
#define OPENAUTO_WIFIMONITOR_HPP

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

namespace f1x::openauto::autoapp::UI {
  class WifiMonitor : public QObject {
  Q_OBJECT

  // TODO: Add in Property for WifiStatus

  public:
    explicit WifiMonitor(QObject *parent = nullptr);

  private slots:

    void onStateChanged(int newState, const QString &interface);

  private:
    void updateIcon(const QString &interface, const QString &color);
  };
}
#endif//OPENAUTO_WIFIMONITOR_HPP