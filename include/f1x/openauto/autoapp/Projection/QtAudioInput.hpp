#pragma once

#include <mutex>
#include <memory>
#include <QObject>
#include <QAudioFormat>
#include <QAudioSource>
#include <QIODevice>

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

    private:
        QAudioFormat audioFormat_;
        QIODevice *ioDevice_;
        std::unique_ptr<QAudioSource> audioInput_;

        ReadPromise::Pointer readPromise_;

        mutable std::mutex mutex_;
        configuration::IConfiguration::Pointer configuration_;

        // Standard chunk size for AA logic (usually 2048 or similar)
        static constexpr size_t cSampleSize = 2048;
    };
}