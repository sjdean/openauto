#pragma once

#include <mutex>
#include <memory>
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QAudioFormat>
#include <QAudioSource>
#include <QIODevice>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

// FFmpeg 5.1 (libavutil 57.28.100) introduced the new Channel Layout API
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
    #define HAS_CH_LAYOUT 1
#else
    #define HAS_CH_LAYOUT 0
#endif

#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

// Forward declaration for AASDK types if not pulled in by IAudioInput
#include <aasdk/IO/Promise.hpp>

namespace f1x::openauto::autoapp::projection {

    class QtAudioInput : public QObject, public IAudioInput {
        Q_OBJECT

    public:
        QtAudioInput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                     configuration::IConfiguration::Pointer config);
        ~QtAudioInput() override;

        bool open() override;

        bool isActive() const override;

        void read(IAudioInput::ReadPromise::Pointer promise) override;

        void start(IAudioInput::StartPromise::Pointer promise) override;

        void stop() override;

        uint32_t getSampleSize() const override;

        uint32_t getChannelCount() const override;

        uint32_t getSampleRate() const override;

        signals:
            // Signal to bridge calls to the Qt thread
            void startRecording(aasdk::io::Promise<void, void>::Pointer promise);
        void stopRecording();

    private slots:
        // Slots running on the GUI/Target thread
        void createAudioInput();

        void onStartRecording(aasdk::io::Promise<void, void>::Pointer promise);

        void onStopRecording();

        void onReadyRead();

        void onAudioStateChanged(QAudio::State state);

    private:
        static AVSampleFormat qAudioFmtToAv(QAudioFormat::SampleFormat fmt);
        bool initSwrContext();
        bool initFilterGraph();

        // requestedFormat_: what AASDK expects (set from constructor params, never changes)
        // audioFormat_:     what QAudioSource actually captures (may be device preferred)
        QAudioFormat requestedFormat_;
        QAudioFormat audioFormat_;
        QIODevice *ioDevice_;
        std::unique_ptr<QAudioSource> audioInput_;
        QTimer *pollTimer_ = nullptr;

        // Software resampler: converts audioFormat_ → requestedFormat_ (null if no resampling needed)
        SwrContext* swrCtx_ = nullptr;

        // Audio filter graph for noise reduction
        AVFilterGraph *filterGraph_ = nullptr;
        AVFilterContext *buffersrcCtx_ = nullptr;
        AVFilterContext *buffersinkCtx_ = nullptr;

        ReadPromise::Pointer readPromise_;

        mutable std::mutex mutex_;
        configuration::IConfiguration::Pointer configuration_;

        // Maximum chunk size per read (in capture-format bytes) — actual reads may be smaller
        static constexpr qint64 cSampleSize = 4096;

        // WAV debug recording (enabled via config "Audio"/"DebugRecord"="true")
        bool debugRecordEnabled_ = false;
        QFile rawWavFile_;
        QFile resampledWavFile_;
        qint64 rawWavDataBytes_       = 0;
        qint64 resampledWavDataBytes_ = 0;

        static void writeWavHeader(QFile &f, int sampleRate, int channels, int bitsPerSample, bool isFloat);
        static void patchWavSizes(QFile &f, qint64 dataBytes);
        void openDebugFiles();
        void closeDebugFiles();
    };
}
