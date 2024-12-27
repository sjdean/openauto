#include <QCoreApplication>
#include <QtBluetooth>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <f1x/openauto/btservice/BluetoothHandler.hpp>
#include <f1x/openauto/btservice/AndroidBluetoothService.hpp>

namespace btservice = f1x::openauto::btservice;

int main(int argc, char *argv[]) {
  QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth*=true"));
  QCoreApplication qApplication(argc, argv);

  auto configuration = std::make_shared<f1x::openauto::autoapp::configuration::Configuration>();

  try {
    auto androidBluetoothService = std::make_shared<btservice::AndroidBluetoothService>();
    btservice::BluetoothHandler bluetoothHandler(androidBluetoothService, configuration);
    QCoreApplication::exec();
  } catch (std::runtime_error& e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }

  OPENAUTO_LOG(info) << "stop";

  return 0;
}
