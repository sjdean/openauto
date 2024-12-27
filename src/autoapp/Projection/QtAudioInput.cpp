#include <f1x/openauto/autoapp/Projection/QtAudioInput.hpp>
#include <QGuiApplication>
#include <QMediaDevices>
#include <f1x/openauto/autoapp/Projection/QtAudioInput.hpp>
#include <f1x/openauto/Common/Log.hpp>


namespace f1x::openauto::autoapp::projection {

  QtAudioInput::QtAudioInput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate)
      : ioDevice_(nullptr) {
    qRegisterMetaType<IAudioInput::StartPromise::Pointer>("StartPromise::Pointer");

    audioFormat_.setChannelCount(channelCount);
    audioFormat_.setSampleRate(sampleRate);

    if (sampleSize == 8) {
      audioFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      audioFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32) {
      audioFormat_.setSampleFormat(QAudioFormat::Int32);
    } // Handle other cases if necessary


    this->moveToThread(QGuiApplication::instance()->thread());
    connect(this, &QtAudioInput::startRecording, this, &QtAudioInput::onStartRecording, Qt::QueuedConnection);
    connect(this, &QtAudioInput::stopRecording, this, &QtAudioInput::onStopRecording, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "createAudioInput", Qt::BlockingQueuedConnection);

  }

  void QtAudioInput::createAudioInput() {
    OPENAUTO_LOG(info) << "[AudioInput] createAudioInput()";
    audioInput_ = (std::make_unique<QAudioSource>(QMediaDevices::defaultAudioInput(), audioFormat_));
  }

  bool QtAudioInput::open() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return ioDevice_ == nullptr;
  }

  bool QtAudioInput::isActive() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return ioDevice_ != nullptr;
  }

  void QtAudioInput::read(ReadPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (ioDevice_ == nullptr) {
      promise->reject();
    } else if (readPromise_ != nullptr) {
      promise->reject();
    } else {
      readPromise_ = std::move(promise);
    }
  }

  void QtAudioInput::start(StartPromise::Pointer promise) {
    emit startRecording(std::move(promise));
  }

  void QtAudioInput::stop() {
    emit stopRecording();
  }

  uint32_t QtAudioInput::getSampleSize() const {
    switch (audioFormat_.sampleFormat()) {
      case QAudioFormat::SampleFormat::UInt8:
        return 8;
      case QAudioFormat::SampleFormat::Int16:
        return 16;
      case QAudioFormat::SampleFormat::Int32:
        return 32;
      default:
        return 0;
    }
  }

  uint32_t QtAudioInput::getChannelCount() const {
    return audioFormat_.channelCount();
  }

  uint32_t QtAudioInput::getSampleRate() const {
    return audioFormat_.sampleRate();
  }

  void QtAudioInput::onStartRecording(StartPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    ioDevice_ = audioInput_->start();

    if (ioDevice_ != nullptr) {
      connect(ioDevice_, &QIODevice::readyRead, this, &QtAudioInput::onReadyRead, Qt::QueuedConnection);
      promise->resolve();
    } else {
      promise->reject();
    }
  }

  void QtAudioInput::onStopRecording() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (readPromise_ != nullptr) {
      readPromise_->reject();
      readPromise_.reset();
    }

    if (ioDevice_ != nullptr) {
      ioDevice_->reset();
      ioDevice_->disconnect();
      ioDevice_ = nullptr;
    }

    audioInput_->stop();
  }

  void QtAudioInput::onReadyRead() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (readPromise_ == nullptr) {
      return;
    }

    aasdk::common::Data data(cSampleSize, 0);
    aasdk::common::DataBuffer buffer(data);
    auto readSize = ioDevice_->read(reinterpret_cast<char *>(buffer.data), buffer.size);

    if (readSize != -1) {
      data.resize(readSize);
      readPromise_->resolve(std::move(data));
      readPromise_.reset();
    } else {
      readPromise_->reject();
      readPromise_.reset();
    }
  }

}



