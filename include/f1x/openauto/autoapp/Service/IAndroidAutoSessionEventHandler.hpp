#pragma once
#include <aasdk/Error/Error.hpp>

namespace f1x::openauto::autoapp::service {
    class IAndroidAutoSessionEventHandler {
    public:
        virtual ~IAndroidAutoSessionEventHandler() = default;

        virtual void onAndroidAutoQuit() = 0;
    };
}
