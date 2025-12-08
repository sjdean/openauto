#include <f1x/openauto/autoapp/Bootstrap/AndroidBluetoothService.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBsBtService, "journeyos.bluetooth.boottrap.service")

namespace f1x::openauto::autoapp::bootstrap {
  /**
   * Stage 1 Bootstrap - Handle iinitial RFCOMM connection to exchange WiFi credentials for Wireless AndroidAuto
   */
  AndroidBluetoothService::AndroidBluetoothService() {
    qInfo(lcBsBtService) << "[AndroidBluetoothService::AndroidBluetoothService] Initialising";
    const QBluetoothUuid serviceUuid(QLatin1String("4de17a00-52cb-11e6-bdf4-0800200c9a66"));

    QBluetoothServiceInfo::Sequence classId;
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    #else
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    #endif

    serviceInfo_.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
    classId.prepend(QVariant::fromValue(serviceUuid));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceName, "OpenAuto Bluetooth Service");
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceDescription, "AndroidAuto WiFi projection automatic setup");
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ServiceProvider, "cubeone.co.uk");
    serviceInfo_.setServiceUuid(serviceUuid);

    QBluetoothServiceInfo::Sequence publicBrowse;

    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
    #else
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
    #endif
    serviceInfo_.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);
  }

  bool AndroidBluetoothService::registerService(int16_t portNumber, const QBluetoothAddress &bluetoothAddress) {
    qInfo(lcBsBtService) << "[AndroidBluetoothService::registerService] Registering Service";

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
    #else
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    #endif

    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();

    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm));
    #else
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
    #endif
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
             << QVariant::fromValue(quint16(portNumber));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo_.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    return serviceInfo_.registerService(bluetoothAddress);
  }

  bool AndroidBluetoothService::unregisterService() {
    qInfo(lcBsBtService) << "[AndroidBluetoothService::unregisterService] Unregistering";
    return serviceInfo_.unregisterService();
  }

}


