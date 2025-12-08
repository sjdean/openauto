#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <memory>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

// Forward declaration if SequentialBuffer isn't used directly in the header,
// but usually it's needed if you used it in the signature.
// Assuming it is not strictly needed for the members below, but keeping your includes clean:
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>

namespace f1x::openauto::autoapp::projection {

    class QtAudioOutput : public QObject, public IAudioOutput {
        Q_OBJECT

    public:
        QtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                      configuration::IConfiguration::Pointer config);

        // -- IAudioOutput Interface Implementation --
        bool open() override;

        void write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer &buffer) override;

        void start() override;

        void stop() override;

        void suspend() override;

        uint32_t getSampleSize() const override;

        uint32_t getChannelCount() const override;

        uint32_t getSampleRate() const override;

        // -- Public Control Methods --
        void setVolume(float volume);

    signals:
        // Internal signals to bridge threads
        void startPlayback();
        void suspendPlayback();
        void stopPlayback();
        void requestSetVolume(float volume); // The signal you were missing

    protected slots:
        // These slots run on the main thread
        void createAudioOutput();

        void onStartPlayback();

        void onSuspendPlayback();

        void onStopPlayback();

        void onSetVolume(float volume);

    private:
        QAudioFormat audioFormat_;
        QIODevice *audioBuffer_; // Weak pointer to internal buffer created by QAudioSink
        std::unique_ptr<QAudioSink> audioOutput_;
        bool playbackStarted_;
        configuration::IConfiguration::Pointer configuration_;
    };
}