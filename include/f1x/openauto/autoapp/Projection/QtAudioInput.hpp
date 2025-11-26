#pragma once
#include <mutex>
#include <QAudioFormat>
#include <QAudioSource>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>

namespace f1x::openauto::autoapp::projection {
    class QtAudioInput : public QObject, public IAudioInput {
        Q_OBJECT

    public:
        QtAudioInput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate);

        bool open() override;

        bool isActive() const override;

        void read(IAudioInput::ReadPromise::Pointer promise) override;

        void start(IAudioInput::StartPromise::Pointer promise) override;

        void stop() override;

        uint32_t getSampleSize() const override;

        uint32_t getChannelCount() const override;

        uint32_t getSampleRate() const override;

    signals:
        void startRecording(aasdk::io::Promise<void, void>::Pointer promise);

        void stopRecording();

    private slots:
        void createAudioInput();

        void onStartRecording(aasdk::io::Promise<void, void>::Pointer promise);

        void onStopRecording();

        void onReadyRead();

    private:
        QAudioFormat audioFormat_;
        QIODevice *ioDevice_;

        std::unique_ptr<QAudioSource> audioInput_;
        ReadPromise::Pointer readPromise_;
        mutable std::mutex mutex_;

        static constexpr size_t cSampleSize = 2056;
    };
}
