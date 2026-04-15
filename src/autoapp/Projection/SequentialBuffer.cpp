#include <QApplication>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcBuffer, "journeyos.buffer")

namespace f1x::openauto::autoapp::projection {

  SequentialBuffer::SequentialBuffer(QObject *parent)
      : QIODevice(parent)
      , m_buffer(MAX_BUFFER_SIZE)
  {
  }

  // Override QIODevice to indicate this is a sequential buffer
  bool SequentialBuffer::isSequential() const {
    return true;
  }

  // Override QIODevice to indicate that this stream is continuous
  bool SequentialBuffer::atEnd() const {
    return false;
  }

  bool SequentialBuffer::open(OpenMode mode) {
    QMutexLocker locker(&m_mutex);
    qInfo(lcBuffer) << "Opening buffer mode:" << mode;

    // Ensure buffer is clear for this open
    m_head = 0;
    m_tail = 0;
    m_size = 0;

    // Open Device in mode required
    return QIODevice::open(mode);
  }

  // readData (Internal pass through from QIODevice)
  qint64 SequentialBuffer::readData(char *data, qint64 maxlen) {
    QMutexLocker locker(&m_mutex);

    // Return 0 if Buffer is Empty
    if (m_size == 0) {
      return 0;
    }

    // Otherwise get length of buffer
    qint64 len = qMin(maxlen, (qint64)m_size);

    // Copy Buffer to Data
    size_t first_part = std::min((size_t)len, MAX_BUFFER_SIZE - m_head);
    std::copy(m_buffer.begin() + m_head, m_buffer.begin() + m_head + first_part, data);

    if (first_part < (size_t)len) {
        size_t second_part = (size_t)len - first_part;
        std::copy(m_buffer.begin(), m_buffer.begin() + second_part, data + first_part);
    }

    // Erase Buffer (advance head)
    m_head = (m_head + len) % MAX_BUFFER_SIZE;
    m_size -= len;

    // Return Length
    return len;
  }

  qint64 SequentialBuffer::size() const {
    return this->bytesAvailable();
  }

  // writeData (Internal pass through to QIODevice
  qint64 SequentialBuffer::writeData(const char *data, qint64 len) {
    QMutexLocker locker(&m_mutex);

    if (len <= 0) return 0;

    // Check space available in the buffer
    qint64 space = MAX_BUFFER_SIZE - m_size;
    if (len > space) {
      // Drop oldest data if full (optional: log warning)
      qWarning(lcBuffer) << "Video buffer overflow! Dropping" << (len - space) << "bytes";
      size_t toDrop = len - space;
      size_t actualDrop = std::min(toDrop, m_size);
      m_head = (m_head + actualDrop) % MAX_BUFFER_SIZE;
      m_size -= actualDrop;
    }

    // We only write up to MAX_BUFFER_SIZE bytes
    size_t writeLen = std::min((size_t)len, MAX_BUFFER_SIZE);
    const char* writeDataPtr = data + (len - writeLen);

    // Insert data into Buffer
    size_t first_part = std::min(writeLen, MAX_BUFFER_SIZE - m_tail);
    std::copy(writeDataPtr, writeDataPtr + first_part, m_buffer.begin() + m_tail);

    if (first_part < writeLen) {
        size_t second_part = writeLen - first_part;
        std::copy(writeDataPtr + first_part, writeDataPtr + writeLen, m_buffer.begin());
    }

    m_tail = (m_tail + writeLen) % MAX_BUFFER_SIZE;
    m_size += writeLen;

    // Notify we have data
    emit readyRead();
    emit bytesWritten(len);

    // Report Length
    return len; // Always report success (or return actual written)
  }

  qint64 SequentialBuffer::pos() const {
    return 0;
  }

  // bytesAvailable
  qint64 SequentialBuffer::bytesAvailable() const {
    QMutexLocker locker(&m_mutex);

    // QIODevice::bytesAvailable() is always null, so all we care about is the size of our internal buffer
    return std::max<qint64>(1, m_size);
  }

  bool SequentialBuffer::canReadLine() const {
    return false;
  }
}
