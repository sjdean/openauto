#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioOutput.hpp>#include <QGuiApplication>
#include <QMediaDevices>
#include <QThread>


namespace f1x::openauto::autoapp::projection {

  QtAudioOutput::QtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate)
      : audioBuffer_(nullptr), playbackStarted_(false) {

    audioFormat_.setChannelCount(channelCount);
    audioFormat_.setSampleRate(sampleRate);

    if (sampleSize == 8) {
      audioFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      audioFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32) {
      audioFormat_.setSampleFormat(QAudioFormat::Int32);
    } // Handle other cases if necessary


    QThread *th = QGuiApplication::instance()->thread();

    this->moveToThread(th);
    th->setPriority(QThread::TimeCriticalPriority);

    connect(this, &QtAudioOutput::startPlayback, this, &QtAudioOutput::onStartPlayback);
    connect(this, &QtAudioOutput::suspendPlayback, this, &QtAudioOutput::onSuspendPlayback);
    connect(this, &QtAudioOutput::stopPlayback, this, &QtAudioOutput::onStopPlayback);

    QMetaObject::invokeMethod(this, "createAudioOutput", Qt::BlockingQueuedConnection);
  }

  void QtAudioOutput::createAudioOutput() {
    OPENAUTO_LOG(info) << "[QtAudioOutput] createAudioOutput()";
    audioOutput_ = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), audioFormat_);
  }

  bool QtAudioOutput::open() {
    return true;
  }

  void QtAudioOutput::write(aasdk::messenger::Timestamp::ValueType, const aasdk::common::DataConstBuffer &buffer) {
    if (audioBuffer_ != nullptr) {
      audioBuffer_->write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
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

  uint32_t QtAudioOutput::getSampleSize() const {
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

  uint32_t QtAudioOutput::getChannelCount() const {
    return audioFormat_.channelCount();
  }

  uint32_t QtAudioOutput::getSampleRate() const {
    return audioFormat_.sampleRate();
  }

  void QtAudioOutput::onStartPlayback() {
    if (!playbackStarted_) {
      audioBuffer_ = audioOutput_->start();
      playbackStarted_ = true;
    } else {
      audioOutput_->resume();
    }
  }

  void QtAudioOutput::onSuspendPlayback() {
    audioOutput_->suspend();
  }

  void QtAudioOutput::onStopPlayback() {
    if (playbackStarted_) {
      audioOutput_->stop();
      playbackStarted_ = false;
    }
  }

}



