#pragma once
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <aap_protobuf/service/inputsource/message/PointerAction.pb.h>

namespace f1x::openauto::autoapp::projection {
    enum class ButtonEventType {
        NONE,
        PRESS,
        RELEASE
    };

    enum class WheelDirection {
        NONE,
        LEFT,
        RIGHT
    };

    struct ButtonEvent {
        ButtonEventType type;
        WheelDirection wheelDirection;
        aap_protobuf::service::media::sink::message::KeyCode code;
    };

    struct TouchEvent {
        aap_protobuf::service::inputsource::message::PointerAction type;
        uint32_t x;
        uint32_t y;
        uint32_t pointerId;
    };
}
