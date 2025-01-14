#pragma once
#include <vector>
#include <memory>
#include <f1x/openauto/Common/Log.hpp>
#include <aap_protobuf/service/control/message/ServiceDiscoveryResponse.pb.h>
#include <aap_protobuf/shared/MessageStatus.pb.h>


namespace f1x::openauto::autoapp::service {

  class IService {
  public:
    typedef std::shared_ptr<IService> Pointer;

    virtual ~IService() = default;

    virtual void start() = 0;

    virtual void stop() = 0;

    virtual void pause() = 0;

    virtual void resume() = 0;

    virtual void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) = 0;
  };

  typedef std::vector<IService::Pointer> ServiceList;

}



