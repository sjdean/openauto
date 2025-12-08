#pragma once
#include <memory>
#include <f1x/openauto/autoapp/Service/IAndroidAutoSessionEventHandler.hpp>

namespace f1x::openauto::autoapp::service {
    class IAndroidAutoSession {
    public:
        typedef std::shared_ptr<IAndroidAutoSession> Pointer;

        virtual ~IAndroidAutoSession() = default;

        virtual void start(IAndroidAutoSessionEventHandler &eventHandler) = 0;

        virtual void stop() = 0;

        virtual void pause() = 0;

        virtual void resume() = 0;
    };
}
