#include <QGuiApplication>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcQtVideoOut, "journeyos.projection.video.qt")

namespace f1x::openauto::autoapp::projection {

  QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration)
      : VideoOutput(std::move(configuration)) {

    // 1. Move to the main GUI thread (Only need to call this once)
    this->moveToThread(QGuiApplication::instance()->thread());

    connect(this, &QtVideoOutput::startPlayback, this, &QtVideoOutput::onStartPlayback, Qt::QueuedConnection);
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::QueuedConnection);

    // Ensure creation happens on the GUI thread
    QMetaObject::invokeMethod(this, "createVideoOutput", Qt::BlockingQueuedConnection);
  }

  void QtVideoOutput::createVideoOutput() {
    qInfo(lcQtVideoOut) << "Initializing Video Widget and Player";

    videoWidget_ = std::make_unique<QVideoWidget>();
    mediaPlayer_ = std::make_unique<QMediaPlayer>();

    // 2. FIX: Connect Signals for proper status reporting (The TODO fix)
    connect(mediaPlayer_.get(), &QMediaPlayer::errorOccurred, this,
        [](QMediaPlayer::Error error, const QString &errorString) {
            qCCritical(lcQtVideoOut) << "Player Error:" << error << "-" << errorString;
    });

    connect(mediaPlayer_.get(), &QMediaPlayer::playbackStateChanged, this,
        [](QMediaPlayer::PlaybackState newState) {
            qCDebug(lcQtVideoOut) << "Playback State Changed:" << newState;
    });

    connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged, this,
        [](QMediaPlayer::MediaStatus status) {
            qCDebug(lcQtVideoOut) << "Media Status Changed:" << status;
    });

    // In Qt6, we set the source to null initially
    mediaPlayer_->setSourceDevice(nullptr);
  }

  bool QtVideoOutput::open() {
    // Ensure buffer is open for Read/Write.
    // AASDK writes to it, MediaPlayer reads from it.
    if (!videoBuffer_.isOpen()) {
        return videoBuffer_.open(QIODevice::ReadWrite);
    }
    return true;
  }

  bool QtVideoOutput::init() {
    emit startPlayback();
    return true;
  }

  void QtVideoOutput::stop() {
    emit stopPlayback();
  }

  void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer &buffer) {
    // WARNING: Ensure videoBuffer_ is thread-safe or this call happens on the correct thread.
    // Standard QBuffer is reentrant but not thread-safe for simultaneous R/W from different threads.
    videoBuffer_.write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
  }

  void QtVideoOutput::onStartPlayback() {
    qInfo(lcQtVideoOut) << "Starting Playback";

    videoWidget_->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoWidget_->setFullScreen(true);
    videoWidget_->show();

    // Ensure the widget has focus to receive key events (if mapped)
    videoWidget_->setFocus();

    mediaPlayer_->setVideoOutput(videoWidget_.get());

    // 3. Robust Buffer Handling
    if (videoBuffer_.isOpen()) {
        // Rewind isn't usually necessary for a stream, but good practice if reusing a fixed buffer
        // videoBuffer_.seek(0);

        mediaPlayer_->setSourceDevice(&videoBuffer_);
        mediaPlayer_->play();
    } else {
        qCCritical(lcQtVideoOut) << "Cannot play: Video buffer is not open.";
    }
  }

  void QtVideoOutput::onStopPlayback() const {
    qInfo(lcQtVideoOut) << "Stopping Playback";

    mediaPlayer_->stop();
    mediaPlayer_->setSourceDevice(nullptr); // Detach buffer

    videoWidget_->hide();
  }

}