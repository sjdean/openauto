#include "f1x/openauto/autoapp/Projection/QtVideoOutput.hpp"
#include <QDebug>
#include <QThread>
#include <QVideoFrameFormat>

namespace f1x::openauto::autoapp::projection {
    QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration, QObject *parent)
        : QObject(parent)
          , VideoOutput(configuration) {
        decodeThread_ = std::thread(&QtVideoOutput::decodeLoop, this);
    }

    QtVideoOutput::~QtVideoOutput() {
        stop();
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
        cv_.notify_all();
    }

    void QtVideoOutput::setVideoSink(QObject *videoOutputItem) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (videoOutputItem == nullptr) {
            qInfo() << "[Video] QML VideoSink detached";
            videoSink_ = nullptr;
            return;
        }

        if (auto sink = qobject_cast<QVideoSink *>(videoOutputItem)) {
            qInfo() << "[Video] QML VideoSink connected —" << sink;
            videoSink_ = sink;
        } else {
            qWarning() << "[Video] setVideoSink: object is NOT a QVideoSink:" << videoOutputItem;
        }
    }

    void QtVideoOutput::write(uint64_t timestamp, const aasdk::common::DataConstBuffer &buffer) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Diagnostic: log first 5 packets and then every 100th
        static int writeCount = 0;
        ++writeCount;
        if (writeCount <= 5 || writeCount % 100 == 0) {
            qDebug() << "[Video] write() #" << writeCount
                     << "size=" << buffer.size
                     << "ts=" << timestamp
                     << "queueLen=" << packetQueue_.size();
        }

        VideoPacket p;
        p.data.assign(buffer.cdata, buffer.cdata + buffer.size);
        p.timestamp = timestamp;

        packetQueue_.push_back(std::move(p));
        cv_.notify_one();
    }

    void QtVideoOutput::decodeLoop() {
        qInfo() << "[Video] decodeLoop starting on thread" << QThread::currentThreadId();

        // 1. Initialize FFmpeg Decoder
        codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec_) {
            qCritical() << "[Video] H.264 decoder NOT found in FFmpeg — cannot decode";
            return;
        }
        qInfo() << "[Video] H.264 decoder found:" << codec_->long_name;

        codecContext_ = avcodec_alloc_context3(codec_);
        codecContext_->flags  |= AV_CODEC_FLAG_LOW_DELAY;
        codecContext_->flags2 |= AV_CODEC_FLAG2_FAST;
        codecContext_->thread_count = 1; // force single-thread for low-latency

        if (avcodec_open2(codecContext_, codec_, nullptr) < 0) {
            qCritical() << "[Video] Could not open H.264 codec context";
            return;
        }
        qInfo() << "[Video] H.264 codec context opened successfully";

        packet_ = av_packet_alloc();
        frame_  = av_frame_alloc();

        int sendCount   = 0;
        int recvCount   = 0;
        bool pixFmtLogged = false;

        while (!stopRequested_) {
            VideoPacket currentPacket;
            QVideoSink* localSink = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !packetQueue_.empty() || stopRequested_.load(); });

                if (stopRequested_) break;

                currentPacket = packetQueue_.front();
                packetQueue_.pop_front();
                localSink = videoSink_;
            }
            // mutex released — write() may queue freely while we decode

            // 2. Prepare AVPacket (points into currentPacket's buffer — alive through this scope)
            packet_->data = currentPacket.data.data();
            packet_->size = static_cast<int>(currentPacket.data.size());
            packet_->pts  = static_cast<int64_t>(currentPacket.timestamp);
            packet_->dts  = static_cast<int64_t>(currentPacket.timestamp);

            // 3. Send to decoder
            int ret = avcodec_send_packet(codecContext_, packet_);
            ++sendCount;

            if (ret < 0) {
                if (ret == AVERROR(EAGAIN)) {
                    // Decoder full — drain first, then retry would be ideal, but
                    // with LOW_DELAY this should be extremely rare; log and skip.
                    qWarning() << "[Video] avcodec_send_packet EAGAIN at packet#" << sendCount
                               << "(decoder needs draining first)";
                } else {
                    char errbuf[AV_ERROR_MAX_STRING_SIZE];
                    av_strerror(ret, errbuf, sizeof(errbuf));
                    qWarning() << "[Video] avcodec_send_packet error at packet#" << sendCount
                               << ":" << errbuf << "(" << ret << ")";
                }
            } else if (sendCount <= 5) {
                qDebug() << "[Video] avcodec_send_packet OK, packet#" << sendCount
                         << "size=" << packet_->size;
            }

            // 4. Receive all available decoded frames
            int receiveRet;
            while ((receiveRet = avcodec_receive_frame(codecContext_, frame_)) == 0) {
                ++recvCount;

                // Log pixel format on first frame (to verify YUV420P assumption)
                if (!pixFmtLogged) {
                    pixFmtLogged = true;
                    qInfo() << "[Video] First frame decoded:"
                            << frame_->width << "x" << frame_->height
                            << "pix_fmt=" << frame_->format
                            << "(YUV420P==" << AV_PIX_FMT_YUV420P << ")"
                            << "pts=" << frame_->pts;
                }

                if (recvCount <= 10 || recvCount % 100 == 0) {
                    qDebug() << "[Video] avcodec_receive_frame #" << recvCount
                             << frame_->width << "x" << frame_->height
                             << "sink=" << (localSink ? "SET" : "NULL");
                }

                if (!localSink) {
                    qDebug() << "[Video] frame#" << recvCount << "discarded — sink not yet connected";
                    continue;
                }

                // Resolve the frame to copy from.
                // YUVJ420P (JPEG/full-range) has identical memory layout to YUV420P — accept directly.
                // Any other format gets converted to YUV420P via a lazy-initialised swscale context.
                const AVFrame* srcFrame = frame_;
                AVFrame* convertedFrame = nullptr;

                if (frame_->format != AV_PIX_FMT_YUV420P && frame_->format != AV_PIX_FMT_YUVJ420P) {
                    if (swsCtxSrcFmt_ != frame_->format) {
                        if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
                        swsCtx_ = sws_getContext(
                            frame_->width, frame_->height, static_cast<AVPixelFormat>(frame_->format),
                            frame_->width, frame_->height, AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
                        swsCtxSrcFmt_ = frame_->format;
                        if (swsCtx_)
                            qInfo() << "[Video] swscale context created: fmt" << frame_->format << "→ YUV420P";
                        else
                            qWarning() << "[Video] sws_getContext failed for fmt" << frame_->format << "— dropping frame";
                    }
                    if (!swsCtx_) { continue; }

                    convertedFrame = av_frame_alloc();
                    convertedFrame->format = AV_PIX_FMT_YUV420P;
                    convertedFrame->width  = frame_->width;
                    convertedFrame->height = frame_->height;
                    av_frame_get_buffer(convertedFrame, 32);
                    sws_scale(swsCtx_,
                              frame_->data,     frame_->linesize,     0, frame_->height,
                              convertedFrame->data, convertedFrame->linesize);
                    srcFrame = convertedFrame;
                }

                QVideoFrameFormat format(QSize(srcFrame->width, srcFrame->height),
                                         QVideoFrameFormat::Format_YUV420P);
                QVideoFrame qFrame(format);

                if (!qFrame.map(QVideoFrame::WriteOnly)) {
                    qWarning() << "[Video] QVideoFrame::map(WriteOnly) failed for"
                               << srcFrame->width << "x" << srcFrame->height;
                    if (convertedFrame) av_frame_free(&convertedFrame);
                    continue;
                }

                // Copy Y plane
                for (int y = 0; y < srcFrame->height; ++y) {
                    memcpy(qFrame.bits(0) + y * qFrame.bytesPerLine(0),
                           srcFrame->data[0] + y * srcFrame->linesize[0],
                           srcFrame->width);
                }
                // Copy U plane
                for (int y = 0; y < srcFrame->height / 2; ++y) {
                    memcpy(qFrame.bits(1) + y * qFrame.bytesPerLine(1),
                           srcFrame->data[1] + y * srcFrame->linesize[1],
                           srcFrame->width / 2);
                }
                // Copy V plane
                for (int y = 0; y < srcFrame->height / 2; ++y) {
                    memcpy(qFrame.bits(2) + y * qFrame.bytesPerLine(2),
                           srcFrame->data[2] + y * srcFrame->linesize[2],
                           srcFrame->width / 2);
                }

                qFrame.unmap();
                qFrame.setStartTime(static_cast<qint64>(srcFrame->pts));

                if (convertedFrame) { av_frame_free(&convertedFrame); convertedFrame = nullptr; }

                if (recvCount <= 3) {
                    qInfo() << "[Video] Calling setVideoFrame on sink" << localSink
                            << "frame#" << recvCount;
                }

                // QVideoSink::setVideoFrame is thread-safe in Qt6.
                localSink->setVideoFrame(qFrame);
            }

            if (receiveRet != AVERROR(EAGAIN) && receiveRet != AVERROR_EOF) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(receiveRet, errbuf, sizeof(errbuf));
                qWarning() << "[Video] avcodec_receive_frame unexpected error:" << errbuf;
            }

            av_packet_unref(packet_);
        }

        qInfo() << "[Video] decodeLoop exiting — sent" << sendCount
                << "packets, decoded" << recvCount << "frames";

        av_packet_free(&packet_);
        av_frame_free(&frame_);
        avcodec_free_context(&codecContext_);
        if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
    }
}
