#include "f1x/openauto/autoapp/Projection/QtVideoOutput.hpp"
#include <QDebug>
#include <QVideoFrameFormat>

namespace f1x::openauto::autoapp::projection {
    QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration, QObject *parent)
        : QObject(parent)
          , VideoOutput(configuration) {
        // [CHANGED] Launch the thread immediately using standard C++
        // This guarantees it runs on a NEW thread, not the GUI thread.
        decodeThread_ = std::thread(&QtVideoOutput::decodeLoop, this);
    }

    QtVideoOutput::~QtVideoOutput() {
        stop();

        // [CHANGED] Join the std::thread
        if (decodeThread_.joinable()) {
            decodeThread_.join();
        }
    }

    bool QtVideoOutput::open() { return true; }
    bool QtVideoOutput::init() { return true; }

    void QtVideoOutput::stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopRequested_ = true;
        }
        cv_.notify_all(); // Wake up the thread so it can exit
    }

    void QtVideoOutput::setVideoSink(QObject *videoOutputItem) {
        std::lock_guard<std::mutex> lock(mutex_); // CRITICAL LOCK

        if (videoOutputItem == nullptr) {
            qInfo() << "QML Video Sync detached";
            videoSink_ = nullptr;
            return;
        }

        if (auto sink = qobject_cast<QVideoSink *>(videoOutputItem)) {
            qInfo() << "QML Video Sync connected";
            videoSink_ = sink;
        }
    }

    void QtVideoOutput::write(uint64_t timestamp, const aasdk::common::DataConstBuffer &buffer) {
        std::lock_guard<std::mutex> lock(mutex_);

        VideoPacket p;
        p.data.assign(buffer.cdata, buffer.cdata + buffer.size);
        p.timestamp = timestamp;

        packetQueue_.push_back(std::move(p));
        cv_.notify_one();
    }

    void QtVideoOutput::decodeLoop() {
        // 1. Initialize FFmpeg Decoder
        codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec_) {
            qCritical() << "H.264 Decoder Not Found!";
            return;
        }

        codecContext_ = avcodec_alloc_context3(codec_);
        codecContext_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codecContext_->flags2 |= AV_CODEC_FLAG2_FAST;

        if (avcodec_open2(codecContext_, codec_, nullptr) < 0) {
            qCritical() << "Could not open H.264 codec";
            return;
        }

        packet_ = av_packet_alloc();
        frame_ = av_frame_alloc();

        while (!stopRequested_) {
            VideoPacket currentPacket;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                // Wait for data OR stop request
                cv_.wait(lock, [this] { return !packetQueue_.empty() || stopRequested_; });

                if (stopRequested_) break;

                currentPacket = packetQueue_.front();
                packetQueue_.pop_front();
            }

            // 2. Prepare Packet
            packet_->data = currentPacket.data.data();
            packet_->size = static_cast<int>(currentPacket.data.size());

            packet_->pts = currentPacket.timestamp;
            packet_->dts = currentPacket.timestamp;

            // 3. Send to Decoder
            int ret = avcodec_send_packet(codecContext_, packet_);
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                // qWarning() << "Decode Error:" << ret;
            }

            // 4. Receive Frames
            while (avcodec_receive_frame(codecContext_, frame_) == 0) {
                if (videoSink_) {
                    QVideoFrameFormat format(QSize(frame_->width, frame_->height),
                                             QVideoFrameFormat::Format_YUV420P);
                    QVideoFrame qFrame(format);

                    if (qFrame.map(QVideoFrame::WriteOnly)) {
                        // Copy Y Plane
                        for (int y = 0; y < frame_->height; ++y) {
                            memcpy(qFrame.bits(0) + y * qFrame.bytesPerLine(0),
                                   frame_->data[0] + y * frame_->linesize[0],
                                   frame_->width);
                        }
                        // Copy U Plane
                        for (int y = 0; y < frame_->height / 2; ++y) {
                            memcpy(qFrame.bits(1) + y * qFrame.bytesPerLine(1),
                                   frame_->data[1] + y * frame_->linesize[1],
                                   frame_->width / 2);
                        }
                        // Copy V Plane
                        for (int y = 0; y < frame_->height / 2; ++y) {
                            memcpy(qFrame.bits(2) + y * qFrame.bytesPerLine(2),
                                   frame_->data[2] + y * frame_->linesize[2],
                                   frame_->width / 2);
                        }

                        qFrame.unmap();
                        qFrame.setStartTime(frame_->pts);

                        std::unique_lock<std::mutex> lock(mutex_);
                        if (videoSink_) {
                            videoSink_->setVideoFrame(qFrame);
                        }
                    }
                }
            }
            av_packet_unref(packet_);
        }

        av_packet_free(&packet_);
        av_frame_free(&frame_);
        avcodec_free_context(&codecContext_);
    }
}
