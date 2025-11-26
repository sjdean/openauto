#pragma once
#include <RtAudio.h>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>

namespace f1x::openauto::autoapp::projection {
    class RtAudioOutput : public IAudioOutput {
    public:
        RtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate);

        bool open() override;

        void write(aasdk::messenger::Timestamp::ValueType timestamp,
                   const aasdk::common::DataConstBuffer &buffer) override;

        void start() override;

        void stop() override;

        void suspend() override;

        uint32_t getSampleSize() const override;

        uint32_t getChannelCount() const override;

        uint32_t getSampleRate() const override;

    private:
        void doSuspend();

        static int audioBufferReadHandler(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                          double streamTime, RtAudioStreamStatus status, void *userData);

        uint32_t channelCount_;
        uint32_t sampleSize_;
        uint32_t sampleRate_;
        SequentialBuffer audioBuffer_;
        std::unique_ptr<RtAudio> dac_;
        std::mutex mutex_;
    };
}
