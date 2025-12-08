#include <aasdk/Transport/SSLWrapper.hpp>
#include <aasdk/Transport/USBTransport.hpp>
#include <aasdk/Transport/TCPTransport.hpp>
#include <aasdk/Messenger/Cryptor.hpp>
#include <aasdk/Messenger/MessageInStream.hpp>
#include <aasdk/Messenger/MessageOutStream.hpp>
#include <aasdk/Messenger/Messenger.hpp>
#include <f1x/openauto/autoapp/Service/SessionFactory.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoSession.hpp>
#include <f1x/openauto/autoapp/Service/Pinger.hpp>

#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcSessionFactory, "journeyos.session.factory")

namespace f1x::openauto::autoapp::service {

  SessionFactory::SessionFactory(boost::asio::io_service &ioService,
                                                     configuration::IConfiguration::Pointer configuration,
                                                     IServiceFactory &serviceFactory,
                                                     std::shared_ptr<UI::Monitor::AndroidAutoMonitor> androidAutoMonitor) :
                                                     ioService_(ioService),
                                                     configuration_(std::move(configuration)),
                                                     serviceFactory_(serviceFactory),
                                                     androidAutoMonitor_(std::move(androidAutoMonitor)) {

  }

  IAndroidAutoSession::Pointer SessionFactory::create(aasdk::usb::IAOAPDevice::Pointer aoapDevice) {
    androidAutoMonitor_->onConnectionStateUpdate(f1x::openauto::common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);
    androidAutoMonitor_->onConnectionMethodUpdate(f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::AA_USB);
    auto transport(std::make_shared<aasdk::transport::USBTransport>(ioService_, std::move(aoapDevice)));
    return create(std::move(transport));
  }

  IAndroidAutoSession::Pointer SessionFactory::create(aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint) {
    androidAutoMonitor_->onConnectionStateUpdate(f1x::openauto::common::Enum::AndroidAutoConnectivityState::AA_CONNECTING);
    androidAutoMonitor_->onConnectionMethodUpdate(f1x::openauto::common::Enum::AndroidAutoConnectivityMethod::AA_WIFI);
    auto transport(std::make_shared<aasdk::transport::TCPTransport>(ioService_, std::move(tcpEndpoint)));
    return create(std::move(transport));
  }

  IAndroidAutoSession::Pointer SessionFactory::create(aasdk::transport::ITransport::Pointer transport) {
    auto sslWrapper(std::make_shared<aasdk::transport::SSLWrapper>());
    auto cryptor(std::make_shared<aasdk::messenger::Cryptor>(std::move(sslWrapper)));
    cryptor->init();

    auto messenger(std::make_shared<aasdk::messenger::Messenger>(ioService_,
                                                                 std::make_shared<aasdk::messenger::MessageInStream>(
                                                                     ioService_, transport, cryptor),
                                                                 std::make_shared<aasdk::messenger::MessageOutStream>(
                                                                     ioService_, transport, cryptor)));

    auto serviceList = serviceFactory_.create(messenger);
    auto pinger(std::make_shared<Pinger>(ioService_, 5000));
    return std::make_shared<AndroidAutoSession>(ioService_, std::move(cryptor), std::move(transport),
                                               std::move(messenger), configuration_, std::move(serviceList),
                                               std::move(pinger), androidAutoMonitor_);
  }

}



