#pragma once
#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QThread> // [ADDED]
#include <memory>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>

namespace f1x::openauto::autoapp::projection {

    class QtAudioOutput : public QObject, public IAudioOutput {
        Q_OBJECT

    public:
        QtAudioOutput(configuration::IConfiguration::Pointer config);
        ~QtAudioOutput() override;

        // -- IAudioOutput Interface --
        void setFormat(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate) override;
        bool open() override;
        void write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer &buffer) override;
        void start() override;
        void stop() override;
        void suspend() override;
        uint32_t getSampleSize() const override;
        uint32_t getChannelCount() const override;
        uint32_t getSampleRate() const override;
        void setVolume(float volume);

        signals:
            void startPlayback();
        void suspendPlayback();
        void stopPlayback();
        void requestSetVolume(float volume);

    protected slots:
        // These run on the worker thread
        void createAudioOutput();
        void onStartPlayback();
        void onSuspendPlayback();
        void onStopPlayback();
        void onSetVolume(float volume);

    private:
        QAudioFormat audioFormat_;
        // [CHANGED] We use our own buffer instance instead of a pointer from QAudioSink
        SequentialBuffer audioInternalBuffer_;

        std::unique_ptr<QAudioSink> audioOutput_;
        bool playbackStarted_;
        configuration::IConfiguration::Pointer configuration_;

        // [ADDED] Dedicated thread for audio hardware
        QThread workerThread_;
    };
}