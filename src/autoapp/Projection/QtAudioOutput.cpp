#include "f1x/openauto/autoapp/Projection/QtAudioOutput.hpp"
#include <QMediaDevices>
#include <QAudioDevice>
#include <algorithm>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcQtAudioOut, "journeyos.projection.audio.qt")

namespace f1x::openauto::autoapp::projection {

  QtAudioOutput::QtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                               configuration::IConfiguration::Pointer config)
      : playbackStarted_(false),
        configuration_(std::move(config)) {

    // 1. Setup Format
    audioFormat_.setChannelCount(channelCount);
    audioFormat_.setSampleRate(sampleRate);

    if (sampleSize == 8) {
      audioFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      audioFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32 || sampleSize == 24) {
      audioFormat_.setSampleFormat(QAudioFormat::Int32);
    } else {
       audioFormat_.setSampleFormat(QAudioFormat::Int16);
    }

    // 2. Prepare Internal Buffer
    // This buffer acts as the safe bridge between Network Thread and Audio Thread.
    audioInternalBuffer_.open(QIODevice::ReadWrite);

    // 3. Threading Magic
    // Move THIS object to the worker thread.
    this->moveToThread(&workerThread_);

    // When thread starts, create the sink (so the sink lives on the thread)
    connect(&workerThread_, &QThread::started, this, &QtAudioOutput::createAudioOutput);

    // Wire up control signals (QueuedConnection is automatic across threads)
    connect(this, &QtAudioOutput::startPlayback, this, &QtAudioOutput::onStartPlayback);
    connect(this, &QtAudioOutput::suspendPlayback, this, &QtAudioOutput::onSuspendPlayback);
    connect(this, &QtAudioOutput::stopPlayback, this, &QtAudioOutput::onStopPlayback);
    connect(this, &QtAudioOutput::requestSetVolume, this, &QtAudioOutput::onSetVolume);

    // 4. Start the Thread
    workerThread_.start();
  }

  QtAudioOutput::~QtAudioOutput() {
      workerThread_.quit();
      workerThread_.wait();
  }

  void QtAudioOutput::createAudioOutput() {
    qInfo(lcQtAudioOut) << "[Threaded] Initializing Sink on:" << QThread::currentThread();

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    QString configDeviceName = configuration_->getSettingByName<QString>("Audio", "PlaybackDevice");
    if(!configDeviceName.isEmpty()) {
        const auto devices = QMediaDevices::audioOutputs();
        for(const auto& d : devices) {
            if(d.description() == configDeviceName) {
                device = d;
                break;
            }
        }
    }

    if (!device.isFormatSupported(audioFormat_)) {
      audioFormat_ = device.preferredFormat();
    }

    // Create Sink on this worker thread
    audioOutput_ = std::make_unique<QAudioSink>(device, audioFormat_);
    audioOutput_->setBufferSize(128000);
  }

  bool QtAudioOutput::open() { return true; }

  void QtAudioOutput::write(aasdk::messenger::Timestamp::ValueType, const aasdk::common::DataConstBuffer &buffer) {
    // [CRITICAL FIX]
    // Write to our memory buffer. This is fast, mutex-protected, and
    // DOES NOT touch the Audio Hardware or Main Thread.
    audioInternalBuffer_.write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
  }

  void QtAudioOutput::start() { emit startPlayback(); }
  void QtAudioOutput::stop() { emit stopPlayback(); }
  void QtAudioOutput::suspend() { emit suspendPlayback(); }
  void QtAudioOutput::setVolume(float volume) { emit requestSetVolume(volume); }

  // -- Slots (Run on Worker Thread) --

  void QtAudioOutput::onStartPlayback() {
    if (!playbackStarted_) {
      qInfo(lcQtAudioOut) << "Stream Start (Pull Mode)";

      // [CRITICAL FIX] PULL MODE
      // We pass our buffer to start(). QAudioSink will now wake up periodically
      // on this worker thread to pull data. It will never block the Main Thread.
      audioOutput_->start(&audioInternalBuffer_);

      float vol = configuration_->getSettingByName<int>("Audio", "PlaybackVolume") / 100.0f;
      audioOutput_->setVolume(std::clamp(vol, 0.0f, 1.0f));
      playbackStarted_ = true;
    } else {
      audioOutput_->resume();
    }
  }

  void QtAudioOutput::onStopPlayback() {
    if (playbackStarted_ && audioOutput_) {
      audioOutput_->stop();
      playbackStarted_ = false;
      // Reset buffer for next time
      audioInternalBuffer_.close();
      audioInternalBuffer_.open(QIODevice::ReadWrite);
    }
  }

  void QtAudioOutput::onSuspendPlayback() {
    if (audioOutput_) audioOutput_->suspend();
  }

  void QtAudioOutput::onSetVolume(float volume) {
      if (audioOutput_) audioOutput_->setVolume(std::clamp(volume, 0.0f, 1.0f));
  }

  uint32_t QtAudioOutput::getSampleSize() const { return 16; } // Simplified
  uint32_t QtAudioOutput::getChannelCount() const { return audioFormat_.channelCount(); }
  uint32_t QtAudioOutput::getSampleRate() const { return audioFormat_.sampleRate(); }
}