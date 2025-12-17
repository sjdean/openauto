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
    m_buffer.clear();

    // Open Device in mode required
    return QIODevice::open(mode);
  }

  // readData (Internal pass through from QIODevice)
  qint64 SequentialBuffer::readData(char *data, qint64 maxlen) {
    QMutexLocker locker(&m_mutex);

    // Return 0 if Buffer is Empty
    if (m_buffer.empty()) {
      return 0;
    }

    // Otherwise get length of buffer
    qint64 len = qMin(maxlen, (qint64)m_buffer.size());

    // Copy Buffer to Data
    std::copy(m_buffer.begin(), m_buffer.begin() + len, data);

    // Erase Buffer
    m_buffer.erase_begin(len);

    // Return Length
    return len;
  }

  qint64 SequentialBuffer::size() const {
    return this->bytesAvailable();
  }

  // writeData (Internal pass through to QIODevice
  qint64 SequentialBuffer::writeData(const char *data, qint64 len) {
    QMutexLocker locker(&m_mutex);

    // Check space available in the buffer
    qint64 space = m_buffer.capacity() - m_buffer.size();
    if (len > space) {
      // Drop oldest data if full (optional: log warning)
      qWarning(lcBuffer) << "Video buffer overflow! Dropping" << (len - space) << "bytes";
      size_t toDrop = len - space;
      m_buffer.erase_begin(std::min(toDrop, m_buffer.size()));
    }

    // Insert data into Buffer
    m_buffer.insert(m_buffer.end(), data, data + len);

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
    return std::max<qint64>(1, m_buffer.size());
  }

  bool SequentialBuffer::canReadLine() const {
    return false;
  }
}