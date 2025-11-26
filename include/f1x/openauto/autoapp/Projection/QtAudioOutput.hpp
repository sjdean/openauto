#pragma once
#include <QAudioFormat>
#include <QAudioSink>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>

namespace f1x::openauto::autoapp::projection {
    class QtAudioOutput : public QObject, public IAudioOutput {
        Q_OBJECT

    public:
        QtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate);

        bool open() override;

        void write(aasdk::messenger::Timestamp::ValueType, const aasdk::common::DataConstBuffer &buffer) override;

        void start() override;

        void stop() override;

        void suspend() override;

        uint32_t getSampleSize() const override;

        uint32_t getChannelCount() const override;

        uint32_t getSampleRate() const override;

    signals:
        void startPlayback();

        void suspendPlayback();

        void stopPlayback();

    protected slots:
        void createAudioOutput();

        void onStartPlayback();

        void onSuspendPlayback();

        void onStopPlayback();

    private:
        QAudioFormat audioFormat_;
        QIODevice *audioBuffer_;
        std::unique_ptr<QAudioSink> audioOutput_;
        bool playbackStarted_;
    };
}
