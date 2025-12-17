#pragma once

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <mutex>
#include <vector>
#include <deque>
#include <thread> // [CHANGED] Include std::thread

// FFmpeg Includes
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include "f1x/openauto/autoapp/Projection/IVideoOutput.hpp"
#include "VideoOutput.hpp"

namespace f1x::openauto::autoapp::projection {

    struct VideoPacket {
        std::vector<uint8_t> data;
        uint64_t timestamp;
    };

    class QtVideoOutput : public QObject, public VideoOutput {
        Q_OBJECT

    public:
        explicit QtVideoOutput(configuration::IConfiguration::Pointer configuration, QObject *parent = nullptr);
        ~QtVideoOutput() override;

        bool open() override;
        bool init() override;
        void stop() override;

        void write(uint64_t timestamp, const aasdk::common::DataConstBuffer &buffer) override;

        Q_INVOKABLE void setVideoSink(QObject* videoOutput);

    private:
        void decodeLoop();

        QVideoSink* videoSink_ = nullptr;

        // [CHANGED] Use std::thread instead of QThread
        std::thread decodeThread_;

        std::mutex mutex_;
        std::condition_variable cv_;
        std::deque<VideoPacket> packetQueue_;
        bool stopRequested_ = false;

        AVCodecContext* codecContext_ = nullptr;
        AVPacket* packet_ = nullptr;
        AVFrame* frame_ = nullptr;
        const AVCodec* codec_ = nullptr;
    };
}