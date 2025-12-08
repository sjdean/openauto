#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBuffer, "journeyos.buffer")

namespace f1x::openauto::autoapp::projection {

  SequentialBuffer::SequentialBuffer()
      : data_(aasdk::common::cStaticDataSize) {
  }

  bool SequentialBuffer::isSequential() const {
    return true;
  }

  bool SequentialBuffer::open(OpenMode mode) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return QIODevice::open(mode);
  }

  qint64 SequentialBuffer::readData(char *data, qint64 maxlen) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (data_.empty()) {
      return 0;
    }

    const auto len = std::min<size_t>(maxlen, data_.size());
    std::copy(data_.begin(), data_.begin() + len, data);
    data_.erase_begin(len);

    return len;
  }

  qint64 SequentialBuffer::writeData(const char *data, qint64 len) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    data_.insert(data_.end(), data, data + len);
    emit readyRead();
    return len;
  }

  qint64 SequentialBuffer::size() const {
    return this->bytesAvailable();
  }

  qint64 SequentialBuffer::pos() const {
    return 0;
  }


  bool SequentialBuffer::seek(qint64) {
    return false;
  }

  bool SequentialBuffer::atEnd() const {
    return false;
  }

  bool SequentialBuffer::reset() {
    data_.clear();
    return true;
  }

  qint64 SequentialBuffer::bytesAvailable() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    return QIODevice::bytesAvailable() + std::max<qint64>(1, data_.size());
  }

  bool SequentialBuffer::canReadLine() const {
    return true;
  }
}



