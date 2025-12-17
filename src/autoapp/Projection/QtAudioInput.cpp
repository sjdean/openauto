#include "f1x/openauto/autoapp/Projection/QtAudioInput.hpp"
#include <QGuiApplication>
#include <QMediaDevices>
#include <QAudioDevice>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcQtAudioIn, "journeyos.projection.audio.input.qt")

namespace f1x::openauto::autoapp::projection {

  QtAudioInput::QtAudioInput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                             configuration::IConfiguration::Pointer config)
      : ioDevice_(nullptr),
        configuration_(std::move(config)) {

    qRegisterMetaType<IAudioInput::StartPromise::Pointer>("StartPromise::Pointer");

    // 1. Setup Format
    audioFormat_.setChannelCount(channelCount);
    audioFormat_.setSampleRate(sampleRate);

    if (sampleSize == 8) {
      audioFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      audioFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32) {
      audioFormat_.setSampleFormat(QAudioFormat::Int32);
    } else {
        // Fallback
        audioFormat_.setSampleFormat(QAudioFormat::Int16);
        qCWarning(lcQtAudioIn) << "Unknown sample size:" << sampleSize << "defaulting to Int16";
    }

    // 2. Thread Safety
   // this->moveToThread(QGuiApplication::instance()->thread());

    connect(this, &QtAudioInput::startRecording, this, &QtAudioInput::onStartRecording, Qt::QueuedConnection);
    connect(this, &QtAudioInput::stopRecording, this, &QtAudioInput::onStopRecording, Qt::QueuedConnection);

    //QMetaObject::invokeMethod(this, "createAudioInput", Qt::BlockingQueuedConnection);
    createAudioInput();
  }

  void QtAudioInput::createAudioInput() {
    qInfo(lcQtAudioIn) << "Initializing Source. Rate:"
                       << audioFormat_.sampleRate()
                       << "Channels:" << audioFormat_.channelCount();

    // 3. Device Selection Logic (Magic)
    QAudioDevice device = QMediaDevices::defaultAudioInput();

    // Check config for a specific microphone name
    QString configDeviceName = configuration_->getSettingByName<QString>("Audio", "CaptureDevice");
    if(!configDeviceName.isEmpty()) {
        const auto devices = QMediaDevices::audioInputs();
        for(const auto& d : devices) {
            if(d.description() == configDeviceName) {
                device = d;
                qInfo(lcQtAudioIn) << "Using configured input device:" << device.description();
                break;
            }
        }
    }

    // 4. Format Check
    if (!device.isFormatSupported(audioFormat_)) {
       qWarning(lcQtAudioIn) << "Requested format not supported. Converting to device preferred format.";
       audioFormat_ = device.preferredFormat();
    }

    audioInput_ = std::make_unique<QAudioSource>(device, audioFormat_);

    // 5. Buffer Magic
    // Set internal buffer to be large enough to hold ~200ms of audio.
    // If the network thread hangs slightly, we won't lose microphone data.
    audioInput_->setBufferSize(32768);

    // 6. Gain/Volume Magic
    // Ensure the mic is "hot". Some OS defaults connect at 0.5 volume.
    audioInput_->setVolume(1.0f);
  }

  bool QtAudioInput::open() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    // In AA protocol, 'open' just means we are ready to start.
    return true;
  }

  bool QtAudioInput::isActive() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return ioDevice_ != nullptr;
  }

  void QtAudioInput::read(ReadPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    // If we aren't running, we can't read.
    if (ioDevice_ == nullptr) {
      promise->reject();
    }
    // If a read is already pending, we can't queue another one in this simple implementation.
    else if (readPromise_ != nullptr) {
      qCWarning(lcQtAudioIn) << "Read request dropped: Previous read still pending.";
      promise->reject();
    }
    else {
      readPromise_ = std::move(promise);

      // OPTIMIZATION: Check immediately if data is ALREADY available in the buffer.
      // This reduces latency by not waiting for the next 'readyRead' signal if data is sitting there.
      if(ioDevice_->bytesAvailable() >= cSampleSize) {
          // Trigger the read logic manually
          // We can't call onReadyRead directly safely because of mutex recursion,
          // but usually onReadyRead locks the mutex itself.
          // Since we hold the lock here, we can't call a function that takes the lock.
          // Best approach: Just rely on the signal, or refactor locking.
          // For safety in this specific architecture, we rely on the signal.
      }
    }
  }

  void QtAudioInput::start(IAudioInput::StartPromise::Pointer promise) {
    Q_EMIT startRecording(std::move(promise));
  }

  void QtAudioInput::stop() {
    Q_EMIT stopRecording();
  }

  uint32_t QtAudioInput::getSampleSize() const {
    switch (audioFormat_.sampleFormat()) {
      case QAudioFormat::UInt8: return 8;
      case QAudioFormat::Int16: return 16;
      case QAudioFormat::Int32: return 32;
      case QAudioFormat::Float: return 32;
      default: return 16;
    }
  }

  uint32_t QtAudioInput::getChannelCount() const {
    return audioFormat_.channelCount();
  }

  uint32_t QtAudioInput::getSampleRate() const {
    return audioFormat_.sampleRate();
  }

  // -- Slots --

  void QtAudioInput::onStartRecording(StartPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    qInfo(lcQtAudioIn) << "Microphone Start";
    ioDevice_ = audioInput_->start();

    if (ioDevice_ != nullptr) {
      connect(ioDevice_, &QIODevice::readyRead, this, &QtAudioInput::onReadyRead, Qt::QueuedConnection);
      promise->resolve();
    } else {
      qCCritical(lcQtAudioIn) << "Failed to start Audio Input Device.";
      promise->reject();
    }
  }

  void QtAudioInput::onStopRecording() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    qInfo(lcQtAudioIn) << "Microphone Stop";

    if (readPromise_ != nullptr) {
      readPromise_->reject();
      readPromise_.reset();
    }

    if (ioDevice_ != nullptr) {
      // Disconnect signals to stop getting readyRead events
      ioDevice_->disconnect(this);
      ioDevice_ = nullptr;
    }

    audioInput_->stop();
  }

  void QtAudioInput::onReadyRead() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    // Logic: AASDK follows a "Pull" model. It asks for data (readPromise_ != null).
    // If AASDK hasn't asked for data yet, we leave it in the QIODevice buffer.
    if (readPromise_ == nullptr) {
      return;
    }

    // Check if we have enough data for a chunk
    // AASDK likes chunks, sending 1 byte packets is inefficient over USB/WiFi.
    qint64 available = ioDevice_->bytesAvailable();
    if(available < (qint64)cSampleSize) {
        return; // Wait for more data
    }

    // Read the chunk
    aasdk::common::Data data(cSampleSize, 0);
    // Qt's read will return however much is available up to cSampleSize
    auto readSize = ioDevice_->read(reinterpret_cast<char *>(data.data()), cSampleSize);

    if (readSize > 0) {
      // Resize vector to actual bytes read if less than requested
      if(readSize < (qint64)cSampleSize) {
          data.resize(readSize);
      }

      readPromise_->resolve(std::move(data));
      readPromise_.reset();
    } else if (readSize == -1) {
        // Error reading
        qCWarning(lcQtAudioIn) << "Error reading from Microphone device.";
        readPromise_->reject();
        readPromise_.reset();
    }
  }
}