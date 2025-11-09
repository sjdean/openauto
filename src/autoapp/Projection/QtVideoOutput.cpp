#include <QGuiApplication>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>

namespace f1x::openauto::autoapp::projection {

  QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration)
      : VideoOutput(std::move(configuration)) {
    this->moveToThread(QGuiApplication::instance()->thread());
    this->moveToThread(QGuiApplication::instance()->thread());

    connect(this, &QtVideoOutput::startPlayback, this, &QtVideoOutput::onStartPlayback, Qt::QueuedConnection);
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "createVideoOutput", Qt::BlockingQueuedConnection);
  }

  void QtVideoOutput::createVideoOutput() {
    OPENAUTO_LOG(info) << "[QtVideoOutput] createVideoOutput()";
    videoWidget_ = std::make_unique<QVideoWidget>();

    // In Qt6, StreamPlayback is not used in the constructor but rather you set the source type explicitly
    mediaPlayer_ = std::make_unique<QMediaPlayer>();
    mediaPlayer_->setSourceDevice(
        nullptr); // This sets up for streaming. Remember to set the actual device when you have it.
  }


  bool QtVideoOutput::open() {
    return videoBuffer_.open(QIODevice::ReadWrite);
  }

  bool QtVideoOutput::init() {
    emit startPlayback();
    return true;
  }

  void QtVideoOutput::stop() {
    emit stopPlayback();
  }

  void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer &buffer) {
    videoBuffer_.write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
  }

  void QtVideoOutput::onStartPlayback() {
    videoWidget_->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoWidget_->setFocus();
    videoWidget_->setFullScreen(true);
    videoWidget_->show();

    mediaPlayer_->setVideoOutput(videoWidget_.get());

    // Use setSource instead of setMedia
    if (&videoBuffer_) {
      mediaPlayer_->setSourceDevice(&videoBuffer_);
    } else {
      // If you have a URL or file, you can use setSource(QUrl)
      // mediaPlayer_->setSource(QUrl::fromLocalFile("path/to/video.mp4"));
    }

    mediaPlayer_->play();

    // TODO: This only outputs a line if there's an error - FIXME - Output a proper status instead
    OPENAUTO_LOG(debug) << "Player error state -> " << mediaPlayer_->errorString().toStdString();
  }

  void QtVideoOutput::onStopPlayback() const {
    videoWidget_->hide();
    mediaPlayer_->stop();
  }

}



