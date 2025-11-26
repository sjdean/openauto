#pragma once

#include <f1x/openauto/autoapp/Projection/InputEvent.hpp>

namespace f1x::openauto::autoapp::projection {
    class IInputDeviceEventHandler {
    public:
        virtual ~IInputDeviceEventHandler() = default;

        virtual void onButtonEvent(const ButtonEvent &event) = 0;

        virtual void onTouchEvent(const TouchEvent &event) = 0;
    };
}
