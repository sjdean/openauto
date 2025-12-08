#include <f1x/openauto/autoapp/Service/Pinger.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcPinger, "journeyos.pinger")

namespace f1x::openauto::autoapp::service {

  Pinger::Pinger(boost::asio::io_service &ioService, time_t duration)
      : strand_(ioService), timer_(ioService), duration_(duration), cancelled_(false), pingsCount_(0), pongsCount_(0) {

  }

  void Pinger::ping(Promise::Pointer promise) {
    strand_.dispatch([this, self = this->shared_from_this(), promise = std::move(promise)]() mutable {
      cancelled_ = false;

      if (promise_ != nullptr) {
        promise_->reject(aasdk::error::Error(aasdk::error::ErrorCode::OPERATION_IN_PROGRESS));
      } else {
        ++pingsCount_;
        qDebug(lcPinger) << "[Pinger] Ping counter: " << pingsCount_;

        promise_ = std::move(promise);
        timer_.expires_from_now(boost::posix_time::milliseconds(duration_));
        timer_.async_wait(
            strand_.wrap(std::bind(&Pinger::onTimerExceeded, this->shared_from_this(), std::placeholders::_1)));
      }
    });
  }

  void Pinger::pong() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      ++pongsCount_;
      qDebug(lcPinger) << "[Pinger] Pong counter: " << pongsCount_;
    });
  }

  void Pinger::onTimerExceeded(const boost::system::error_code &error) {
    if (promise_ == nullptr) {
      return;
    } else if (error == boost::asio::error::operation_aborted || cancelled_) {
      promise_->reject(aasdk::error::Error(aasdk::error::ErrorCode::OPERATION_ABORTED));
    } else if (pingsCount_ - pongsCount_ > 4) {
      promise_->reject(aasdk::error::Error());
    } else {
      promise_->resolve();
    }

    promise_.reset();
  }

  void Pinger::cancel() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      cancelled_ = true;
      timer_.cancel();
    });
  }

}



