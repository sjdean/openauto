
#include <boost/algorithm/hex.hpp>
#include <QNetworkInterface>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/unknown_field_set.h>
#include <aap_protobuf/service/control/message/AudioFocusRequestType.pb.h>
#include <aap_protobuf/service/wifiprojection/WifiProjectionService.pb.h>
#include <aap_protobuf/service/wifiprojection/message/WifiCredentialsRequest.pb.h>
#include <aap_protobuf/service/wifiprojection/message/AccessPointType.pb.h>
#include <aap_protobuf/service/wifiprojection/message/WifiSecurityMode.pb.h>
#include "f1x/openauto/autoapp/Bootstrap/AndroidBluetoothServer.hpp"
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "aaw/WifiVersionRequest.pb.h"
#include "aaw/WifiVersionResponse.pb.h"
#include "aaw/WifiInfoRequest.pb.h"
#include "aaw/WifiInfoResponse.pb.h"
#include "aaw/WifiStartRequest.pb.h"
#include "aaw/WifiStartResponse.pb.h"
#include "aaw/WifiConnectionStatus.pb.h"

using namespace google::protobuf;
using namespace google::protobuf::io;

// 39171FDJG002WHhandleWifiVersionRequest
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBsBtServer, "journeyos.bluetooth.bootstrap.server")

namespace f1x::openauto::autoapp::bootstrap {
using configuration::ConfigGroup;
using configuration::ConfigKey;

  /**
   * Stage 1 Bootstrap - Handle iinitial RFCOMM connection to exchange WiFi credentials for Wireless AndroidAuto
   * @param configuration
   */
  AndroidBluetoothServer::AndroidBluetoothServer(configuration::IConfiguration::Pointer configuration)
      : rfcommServer_(std::make_unique<QBluetoothServer>(QBluetoothServiceInfo::RfcommProtocol, this)),
        configuration_(std::move(configuration)) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::AndroidBluetoothServer] Initialising";

    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this,
            &AndroidBluetoothServer::onClientConnected);

  }

  /// Start Server listening on Address
  uint16_t AndroidBluetoothServer::start(const QBluetoothAddress &address) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::start] Attempting to listen on address:"
                        << (address.isNull() ? "(any)" : address.toString());
    if (rfcommServer_->listen(address)) {
      qInfo(lcBsBtServer) << "[AndroidBluetoothServer::start] Listening on RFCOMM port:"
                          << rfcommServer_->serverPort();
      return rfcommServer_->serverPort();
    }
    qCritical(lcBsBtServer) << "[AndroidBluetoothServer::start] listen() failed — RFCOMM server not started";
    return 0;
  }

  void AndroidBluetoothServer::onError(QBluetoothServer::Error error) {
    qCritical(lcBsBtServer) << "[AndroidBluetoothServer::onError] RFCOMM server error:" << static_cast<int>(error);
  }

// ---------------------------------------------------------
  // HELPER: Find the Network Interface based on Configured MAC
  // ---------------------------------------------------------
  QNetworkInterface AndroidBluetoothServer::findConfiguredInterface() const {
      // 1. Get the target MAC address from settings.
      // "InterfaceMAC" stores the hardware address (e.g. "B8:27:EB:...").
      // "Interface" stores the interface name and must not be used for MAC comparison.
      QString targetMac = configuration_->getSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessInterfaceMAC);

      // Normalize config MAC (remove dashes/colons, make uppercase) for safer comparison
      QString normalizedTarget = targetMac.toUpper();

      // 2. Iterate all system interfaces
      const auto interfaces = QNetworkInterface::allInterfaces();
      for (const auto &interface : interfaces) {
          // 3. Compare Hardware Address
          if (interface.hardwareAddress().toUpper() == normalizedTarget) {
              qCDebug(lcBsBtServer) << "Found configured interface:" << interface.name() << "MAC:" << normalizedTarget;
              return interface;
          }
      }

      // 4. Fallback (if config is empty or device missing, try wlan0 or lo)
      qCWarning(lcBsBtServer) << "Configured MAC" << targetMac << "not found. Falling back to 'wlan0'";
      return QNetworkInterface::interfaceFromName("wlan0");
  }

  /// Call-Back for when Client Connected
  void AndroidBluetoothServer::onClientConnected() {
    qDebug(lcBsBtServer) << "[AndroidBluetoothServer::onClientConnected]";
    if (socket != nullptr) {
      socket->deleteLater();
    }

    socket = rfcommServer_->nextPendingConnection();

    if (socket != nullptr) {
      qInfo(lcBsBtServer) << "[AndroidBluetoothServer] RFCOMM client connected"
                          << "peer:" << socket->peerName()
                          << "addr:" << socket->peerAddress().toString();

      connect(socket, &QBluetoothSocket::readyRead, this, &AndroidBluetoothServer::readSocket);
      connect(socket, &QBluetoothSocket::disconnected, this, [this]() {
        qInfo(lcBsBtServer) << "[AndroidBluetoothServer] RFCOMM socket disconnected";
      });
      connect(socket, &QBluetoothSocket::stateChanged, this, [this](QBluetoothSocket::SocketState state) {
        qInfo(lcBsBtServer) << "[AndroidBluetoothServer] socket state ->" << state;
      });
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      connect(socket, &QBluetoothSocket::errorOccurred, this, [this](QBluetoothSocket::SocketError err) {
        qCritical(lcBsBtServer) << "[AndroidBluetoothServer] RFCOMM socket error:" << err;
      });
#else
      connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
              this, [this](QBluetoothSocket::SocketError err) {
        qCritical(lcBsBtServer) << "[AndroidBluetoothServer] RFCOMM socket error:" << err;
      });
#endif

      aap_protobuf::aaw::WifiVersionRequest versionRequest;
      aap_protobuf::aaw::WifiStartRequest startRequest;

      QNetworkInterface wifiInterface = findConfiguredInterface();
      std::string ipAddress = "";

      // Loop through address entries to find IPv4
      for (const QNetworkAddressEntry &entry : wifiInterface.addressEntries()) {
          if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback()) {
              ipAddress = entry.ip().toString().toStdString();
              break;
          }
      }

      if (ipAddress.empty()) {
        qCCritical(lcBsBtServer) << "Could not find IPv4 address for interface:" << wifiInterface.name();
        // Fallback or error handling here?
      } else {
        qCInfo(lcBsBtServer) << "Advertising WiFi IP:" << ipAddress.c_str();
      }

      startRequest.set_ip_address(ipAddress);
      startRequest.set_port(5000);

      sendMessage(versionRequest, aap_protobuf::aaw::MessageId::WIFI_VERSION_REQUEST);
      sendMessage(startRequest, aap_protobuf::aaw::MessageId::WIFI_START_REQUEST);
    } else {
      qCritical(lcBsBtServer) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
  }

  /// Read data from Bluetooth Socket
  void AndroidBluetoothServer::readSocket() {
    buffer += socket->readAll();

    qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Reading from socket.";

    if (buffer.length() < 4) {
      qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Not enough data, waiting for more.";
      return;
    }

    QDataStream stream(buffer);
    uint16_t length;
    stream >> length;

    if (buffer.length() < length + 4) {
      qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Not enough data, waiting for more: " << buffer.length();
      return;
    }

    quint16 rawMessageId;
    stream >> rawMessageId;

    aap_protobuf::aaw::MessageId messageId;
    messageId = static_cast<aap_protobuf::aaw::MessageId>(rawMessageId);

    qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Message length: " << length << " MessageId: " << messageId;

    switch (messageId) {

      case aap_protobuf::aaw::MessageId::WIFI_INFO_REQUEST: // WifiInfoRequest - Respond with a WifiInfoResponse
        handleWifiInfoRequest(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_VERSION_RESPONSE: // WifiVersionRequest - Send a Version Request
        handleWifiVersionResponse(buffer, length);// do something
        break;
      case aap_protobuf::aaw::MessageId::WIFI_CONNECTION_STATUS: // WifiStartResponse  - Receive a confirmation
        handleWifiConnectionStatus(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_START_RESPONSE: // WifiStartResponse  - Receive a confirmation
        handleWifiStartResponse(buffer, length);
        break;
      case aap_protobuf::aaw::MessageId::WIFI_START_REQUEST:      // These are not received from the MD.
      case aap_protobuf::aaw::MessageId::WIFI_INFO_RESPONSE:      // These are not received from the MD.
      case aap_protobuf::aaw::MessageId::WIFI_VERSION_REQUEST:    // These are not received from the MD.
      default:
        QByteArray messageData = buffer.mid(stream.device()->pos(), length - 2);

        // Convert QByteArray to std::string
        std::string protoData = messageData.toStdString();

        // Pass it to your function
        this->DecodeProtoMessage(protoData);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto &&val: buffer) {
          ss << std::setw(2) << static_cast<unsigned>(val);
        }
        qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Unknown message: " << messageId;
        qDebug(lcBsBtServer) << "[AndroidBluetoothServer::readSocket] Data " << ss.str();

        break;
    }

    buffer = buffer.mid(length + 4);
  }

  /// Handles request for WifiInfoRequest by sending a WifiInfoResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiInfoRequest(QByteArray &buffer, uint16_t length) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiInfoRequest] Received WifiInfoRequest from phone";

    aap_protobuf::aaw::WifiInfoResponse response;

    const QString ssid = configuration_->getSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessHotspotSSID);
    const QString password = configuration_->getSettingByName<QString>(ConfigGroup::Wireless, ConfigKey::WirelessHotspotPassword);

    if (ssid.isEmpty()) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiInfoRequest] HotspotSSID is empty — phone will fail to connect";
    }
    if (password.isEmpty()) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiInfoRequest] HotspotPassword is empty";
    }

    response.set_ssid(ssid.toStdString());
    response.set_password(password.toStdString());

    QNetworkInterface wifiInterface = findConfiguredInterface();

    // BSSID is essentially the MAC address of the AP
    std::string bssid = wifiInterface.hardwareAddress().toStdString();

    if (bssid.empty()) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiInfoRequest] Failed to resolve BSSID (MAC) for interface:" << wifiInterface.name();
    } else {
      qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiInfoRequest] Advertising:"
                          << "interface=" << wifiInterface.name()
                          << "SSID=" << ssid
                          << "BSSID=" << bssid.c_str()
                          << "security=WPA2_PERSONAL";
    }

    response.set_bssid(bssid);
    response.set_security_mode(
        aap_protobuf::service::wifiprojection::message::WifiSecurityMode::WPA2_PERSONAL);
    response.set_access_point_type(aap_protobuf::service::wifiprojection::message::AccessPointType::STATIC);

    sendMessage(response, 3);
  }

  /// Listens for a WifiVersionResponse from the MD - usually just a notification
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiVersionResponse(QByteArray &buffer, uint16_t length) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiVersionResponse] Handling wifi version response";
    aap_protobuf::aaw::WifiVersionResponse response;
    response.ParseFromArray(buffer.data() + 4, length);qDebug(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiVersionResponse] Unknown Param 1: " << response.major_version() << " Unknown Param 2: " << response.minor_version();
  }

  /// Listens for WifiStartResponse from MD - usually just a notification with a status
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiStartResponse(QByteArray &buffer, uint16_t length) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiStartResponse] Received WifiStartResponse from phone";
    aap_protobuf::aaw::WifiStartResponse response;
    if (!response.ParseFromArray(buffer.data() + 4, length)) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiStartResponse] Failed to parse WifiStartResponse protobuf";
      return;
    }
    const auto statusName = Status_Name(response.status());
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiStartResponse]"
                        << "phone connecting to ip=" << response.ip_address().c_str()
                        << "port=" << response.port()
                        << "status=" << statusName.c_str();
    if (response.status() != aap_protobuf::aaw::STATUS_SUCCESS) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiStartResponse] Phone reported non-success status — wireless session may not start";
    }
  }

  /// Handles request for WifiStartRequest by sending a WifiStartResponse
  /// \param buffer
  /// \param length
  void AndroidBluetoothServer::handleWifiConnectionStatus(QByteArray &buffer, uint16_t length) {
    aap_protobuf::aaw::WifiConnectionStatus connStatus;
    if (!connStatus.ParseFromArray(buffer.data() + 4, length)) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiConnectionStatus] Failed to parse WifiConnectionStatus protobuf";
      return;
    }
    const auto statusName = Status_Name(connStatus.status());
    if (connStatus.status() == aap_protobuf::aaw::STATUS_SUCCESS) {
      qInfo(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiConnectionStatus] Phone WiFi connection status: SUCCESS";
    } else {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::handleWifiConnectionStatus] Phone WiFi connection status:"
                              << statusName.c_str()
                              << (connStatus.has_error_message() ? ("— " + connStatus.error_message()).c_str() : "");
    }
  }

  void AndroidBluetoothServer::sendMessage(const google::protobuf::Message &message, uint16_t type) {
    qInfo(lcBsBtServer) << "[AndroidBluetoothServer::sendMessage] type=" << type
                        << "proto=" << std::string(message.GetTypeName()).c_str();

    int byteSize = message.ByteSizeLong();
    QByteArray out(byteSize + 4, 0);
    QDataStream ds(&out, QIODevice::ReadWrite);
    ds << (uint16_t) byteSize;
    ds << type;
    message.SerializeToArray(out.data() + 4, byteSize);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto &&val: out) {
      ss << std::setw(2) << static_cast<unsigned>(val);
    }

    qDebug(lcBsBtServer) << message.GetTypeName() << " - " + message.DebugString();

    auto written = socket->write(out);
    if (written > -1) {
      qDebug(lcBsBtServer) << "[AndroidBluetoothServer::sendMessage] Bytes written: " << written;
    } else {
      qDebug(lcBsBtServer) << "[AndroidBluetoothServer::sendMessage] Could not write data";
    }
  }

  const ::std::string AndroidBluetoothServer::getIP4_(const QString intf) {
    for (const QNetworkAddressEntry &address: QNetworkInterface::interfaceFromName(intf).addressEntries()) {
      if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
        return address.ip().toString().toStdString();
    }
    return "";
  }

  /// Decode Proto Messages to their constituent components
  /// \param proto_data
  void AndroidBluetoothServer::DecodeProtoMessage(const std::string& proto_data) {
    UnknownFieldSet set;

    ArrayInputStream raw_input(proto_data.data(), proto_data.size());
    CodedInputStream input(&raw_input);

    if (!set.MergeFromCodedStream(&input)) {
      qCWarning(lcBsBtServer) << "[AndroidBluetoothServer::DecodeProtoMessage] Failed to decode unknown message";
      return;
    }

    for (int i = 0; i < set.field_count(); ++i) {
      const UnknownField& field = set.field(i);
      switch (field.type()) {
        case UnknownField::TYPE_VARINT:
          qDebug(lcBsBtServer) << "[DecodeProtoMessage] field" << field.number() << "varint:" << field.varint();
          break;
        case UnknownField::TYPE_FIXED32:
          qDebug(lcBsBtServer) << "[DecodeProtoMessage] field" << field.number() << "fixed32:" << field.fixed32();
          break;
        case UnknownField::TYPE_FIXED64:
          qDebug(lcBsBtServer) << "[DecodeProtoMessage] field" << field.number() << "fixed64:" << field.fixed64();
          break;
        case UnknownField::TYPE_LENGTH_DELIMITED: {
          const std::string ld(field.length_delimited());
          std::stringstream ss;
          ss << std::hex << std::setfill('0');
          for (unsigned char ch : ld) { ss << std::setw(2) << static_cast<unsigned>(ch); }
          qDebug(lcBsBtServer) << "[DecodeProtoMessage] field" << field.number() << "length_delimited:" << ss.str().c_str();
          break;
        }
        case UnknownField::TYPE_GROUP:
          qDebug(lcBsBtServer) << "[DecodeProtoMessage] field" << field.number() << "group (deprecated)";
          break;
      }
    }
  }
}

