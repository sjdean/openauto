#include "f1x/openauto/autoapp/Projection/QtAudioOutput.hpp"

#include <QGuiApplication>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QThread>
#include <algorithm> // for std::clamp
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcQtAudioOut, "journeyos.projection.audio.qt")

namespace f1x::openauto::autoapp::projection {

  QtAudioOutput::QtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                               configuration::IConfiguration::Pointer config)
      : audioBuffer_(nullptr),
        playbackStarted_(false),
        configuration_(std::move(config)) {

    // 1. Setup Audio Format
    audioFormat_.setChannelCount(channelCount);
    audioFormat_.setSampleRate(sampleRate);

    // Map AASDK sample sizes to Qt6 Format
    if (sampleSize == 8) {
      audioFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      audioFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32 || sampleSize == 24) {
      // 24-bit is usually handled as Int32 in Qt alignment
      audioFormat_.setSampleFormat(QAudioFormat::Int32);
    } else {
       // Fallback for weird formats
       audioFormat_.setSampleFormat(QAudioFormat::Int16);
       qCWarning(lcQtAudioOut) << "Unknown sample size:" << sampleSize << "defaulting to Int16";
    }

    // 2. Thread Safety Logic
    // We move this object to the GUI thread so signals/slots work correctly.
    // We DO NOT set the GUI thread to TimeCritical (that freezes the UI).
    this->moveToThread(QGuiApplication::instance()->thread());

    // Connect internal signals to slots (QueuedConnection handles the thread jump)
    connect(this, &QtAudioOutput::startPlayback, this, &QtAudioOutput::onStartPlayback, Qt::QueuedConnection);
    connect(this, &QtAudioOutput::suspendPlayback, this, &QtAudioOutput::onSuspendPlayback, Qt::QueuedConnection);
    connect(this, &QtAudioOutput::stopPlayback, this, &QtAudioOutput::onStopPlayback, Qt::QueuedConnection);
    connect(this, &QtAudioOutput::requestSetVolume, this, &QtAudioOutput::onSetVolume, Qt::QueuedConnection);

    // Create the actual AudioSink on the target thread
    QMetaObject::invokeMethod(this, "createAudioOutput", Qt::BlockingQueuedConnection);
  }

  void QtAudioOutput::createAudioOutput() {
    qInfo(lcQtAudioOut) << "[QtAudioOutput] Initializing Sink. Rate:"
                        << audioFormat_.sampleRate()
                        << "Channels:" << audioFormat_.channelCount();

    // 3. Device Selection
    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    // Check if user specified a device in config
    QString configDeviceName = configuration_->getSettingByName<QString>("Audio", "OutputDevice");
    if(!configDeviceName.isEmpty()) {
        const auto devices = QMediaDevices::audioOutputs();
        for(const auto& d : devices) {
            if(d.description() == configDeviceName) {
                device = d;
                qInfo(lcQtAudioOut) << "Using configured audio device:" << device.description();
                break;
            }
        }
    }

    // 4. Format Verification
    if (!device.isFormatSupported(audioFormat_)) {
      qWarning(lcQtAudioOut) << "Requested format not supported. Converting to device preferred format.";
      audioFormat_ = device.preferredFormat();
    }

    // 5. Create Sink
    audioOutput_ = std::make_unique<QAudioSink>(device, audioFormat_);

    // 6. Magic Fix: Set Buffer Size
    // Default buffer might be too small for network streams, causing pops.
    // 128000 bytes is roughly ~100-200ms of audio depending on bitrate.
    audioOutput_->setBufferSize(128000);

    // Optional: Log state changes for debugging
    connect(audioOutput_.get(), &QAudioSink::stateChanged, this, [](QAudio::State newState){
        if (newState == QAudio::IdleState) {
             // qCDebug(lcQtAudioOut) << "Audio Underrun (Idle)";
        } else if (newState == QAudio::StoppedState) {
             // qCDebug(lcQtAudioOut) << "Audio Stopped";
        }
    });
  }

  bool QtAudioOutput::open() {
    // AASDK calls this. We are ready as soon as the constructor finishes.
    return true;
  }

  void QtAudioOutput::write(aasdk::messenger::Timestamp::ValueType, const aasdk::common::DataConstBuffer &buffer) {
    // Called from USB/Network thread.
    // audioBuffer_ is the QIODevice returned by QAudioSink::start().
    // QIODevice::write is generally reentrant, but we check existence first.
    if (audioBuffer_ != nullptr && playbackStarted_) {
      qint64 written = audioBuffer_->write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
      if(written != buffer.size) {
          qCWarning(lcQtAudioOut) << "Audio buffer full. Dropped bytes.";
      }
    }
  }

  void QtAudioOutput::start() {
    emit startPlayback();
  }

  void QtAudioOutput::stop() {
    emit stopPlayback();
  }

  void QtAudioOutput::suspend() {
    emit suspendPlayback();
  }

  void QtAudioOutput::setVolume(float volume) {
    // Public API calls this, which emits the signal to the correct thread
    emit requestSetVolume(volume);
  }

  uint32_t QtAudioOutput::getSampleSize() const {
    switch (audioFormat_.sampleFormat()) {
      case QAudioFormat::UInt8: return 8;
      case QAudioFormat::Int16: return 16;
      case QAudioFormat::Int32: return 32;
      case QAudioFormat::Float: return 32;
      default: return 16;
    }
  }

  uint32_t QtAudioOutput::getChannelCount() const {
    return audioFormat_.channelCount();
  }

  uint32_t QtAudioOutput::getSampleRate() const {
    return audioFormat_.sampleRate();
  }

  // -- Slots (Run on GUI Thread) --

  void QtAudioOutput::onStartPlayback() {
    if (!playbackStarted_) {
      qInfo(lcQtAudioOut) << "Stream Start";
      // start() returns the QIODevice we write to
      audioBuffer_ = audioOutput_->start();

      // Set initial volume
      float vol = configuration_->getSettingByName<int>("Audio", "MusicVolume") / 100.0f;
      // Clamp just in case
      vol = std::max(0.0f, std::min(1.0f, vol));
      audioOutput_->setVolume(vol);

      playbackStarted_ = true;
    } else {
      audioOutput_->resume();
    }
  }

  void QtAudioOutput::onSuspendPlayback() {
    if (audioOutput_) {
        audioOutput_->suspend();
    }
  }

  void QtAudioOutput::onStopPlayback() {
    if (playbackStarted_ && audioOutput_) {
      qInfo(lcQtAudioOut) << "Stream Stop";
      audioOutput_->stop();
      playbackStarted_ = false;
      audioBuffer_ = nullptr;
    }
  }

  void QtAudioOutput::onSetVolume(float volume) {
      if (audioOutput_) {
          // Clamp volume between 0.0 and 1.0
          float safeVolume = std::max(0.0f, std::min(1.0f, volume));
          audioOutput_->setVolume(safeVolume);
      }
  }

}