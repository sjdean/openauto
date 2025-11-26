#pragma once

#include <QRect>
#include <aasdk/IO/Promise.hpp>
#include <f1x/openauto/autoapp/Projection/InputEvent.hpp>

namespace f1x::openauto::autoapp::projection {
    class IInputDeviceEventHandler;

    class IInputDevice {
    public:
        typedef std::shared_ptr<IInputDevice> Pointer;
        typedef std::vector<aap_protobuf::service::media::sink::message::KeyCode> ButtonCodes;

        virtual ~IInputDevice() = default;

        virtual void start(IInputDeviceEventHandler &eventHandler) = 0;

        virtual void stop() = 0;

        virtual ButtonCodes getSupportedButtonCodes() const = 0;

        virtual bool hasTouchscreen() const = 0;

        virtual QRect getTouchscreenGeometry() const = 0;
    };
}
