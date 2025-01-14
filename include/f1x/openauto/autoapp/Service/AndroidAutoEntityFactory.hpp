#pragma once

#include <boost/asio.hpp>
#include <aasdk/Transport/ITransport.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>
#include <f1x/openauto/autoapp/Service/IServiceFactory.hpp>
#include <f1x/openauto/autoapp/UI/AndroidAutoMonitor.hpp>

namespace f1x::openauto::autoapp::service {

  class AndroidAutoEntityFactory : public IAndroidAutoEntityFactory {
  public:
    AndroidAutoEntityFactory(boost::asio::io_service &ioService,
                             configuration::IConfiguration::Pointer configuration,
                             IServiceFactory &serviceFactory,
                             std::shared_ptr<UI::AndroidAutoMonitor> androidAutoMonitor);

    IAndroidAutoEntity::Pointer create(aasdk::usb::IAOAPDevice::Pointer aoapDevice) override;

    IAndroidAutoEntity::Pointer create(aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint) override;

  private:
    IAndroidAutoEntity::Pointer create(aasdk::transport::ITransport::Pointer transport);

    boost::asio::io_service &ioService_;
    configuration::IConfiguration::Pointer configuration_;
    IServiceFactory &serviceFactory_;
    std::shared_ptr<UI::AndroidAutoMonitor> androidAutoMonitor_;
  };

}



