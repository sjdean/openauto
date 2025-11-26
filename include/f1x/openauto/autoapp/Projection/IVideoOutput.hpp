#pragma once
#include <memory>
#include <QRect>
#include <aasdk/Common/Data.hpp>
#include <aasdk/Messenger/Timestamp.hpp>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>

namespace f1x::openauto::autoapp::projection {
    class IVideoOutput {
    public:
        typedef std::shared_ptr<IVideoOutput> Pointer;

        IVideoOutput() = default;

        virtual ~IVideoOutput() = default;

        virtual bool open() = 0;

        virtual bool init() = 0;

        virtual void write(aasdk::messenger::Timestamp::ValueType timestamp,
                           const aasdk::common::DataConstBuffer &buffer) = 0;

        virtual void stop() = 0;

        virtual aap_protobuf::service::media::sink::message::VideoFrameRateType getVideoFPS() const = 0;

        virtual aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() const = 0;

        virtual size_t getScreenDPI() const = 0;

        virtual QRect getVideoMargins() const = 0;
    };
}
