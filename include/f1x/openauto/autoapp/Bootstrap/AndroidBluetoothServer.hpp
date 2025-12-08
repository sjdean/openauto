#pragma once
#include <stdint.h>
#include <memory>
#include <QBluetoothServer>
#include <QNetworkInterface>
#include <aap_protobuf/aaw/MessageId.pb.h>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Bootstrap/IAndroidBluetoothServer.hpp>

namespace f1x::openauto::autoapp::bootstrap {
    class AndroidBluetoothServer : public QObject, public IAndroidBluetoothServer {
        Q_OBJECT

    public:
        AndroidBluetoothServer(configuration::IConfiguration::Pointer configuration);

        uint16_t start(const QBluetoothAddress &address) override;

    private slots:
        void onClientConnected();

        void onError(QBluetoothServer::Error error);

    private:

        QNetworkInterface findConfiguredInterface() const;

        void readSocket();

        void handleWifiInfoRequest(QByteArray &buffer, uint16_t length);

        void handleWifiVersionResponse(QByteArray &buffer, uint16_t length);

        void handleWifiConnectionStatus(QByteArray &buffer, uint16_t length);

        void handleWifiStartResponse(QByteArray &buffer, uint16_t length);

        void sendMessage(const google::protobuf::Message &message, uint16_t type);

        const ::std::string getIP4_(const QString intf);

        void DecodeProtoMessage(const std::string &proto_data);

        std::unique_ptr<QBluetoothServer> rfcommServer_;
        QBluetoothSocket *socket = nullptr;
        configuration::IConfiguration::Pointer configuration_;
        QByteArray buffer;
    };
}
