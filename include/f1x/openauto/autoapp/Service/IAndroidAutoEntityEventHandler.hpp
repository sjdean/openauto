#pragma once
#include <aasdk/Error/Error.hpp>

namespace f1x::openauto::autoapp::service {
    class IAndroidAutoEntityEventHandler {
    public:
        virtual ~IAndroidAutoEntityEventHandler() = default;

        virtual void onAndroidAutoQuit() = 0;
    };
}
