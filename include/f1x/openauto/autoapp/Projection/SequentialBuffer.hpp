#pragma once
#include <QIODevice>
#include <mutex>
#include <QMutex>
#include <qwaitcondition.h>
#include <vector>
#include <aasdk/Common/Data.hpp>

namespace f1x::openauto::autoapp::projection {
    class SequentialBuffer : public QIODevice {
        Q_OBJECT
    public:
        explicit SequentialBuffer(QObject *parent = nullptr);

        bool isSequential() const override;

        qint64 pos() const override;

        bool canReadLine() const override;

        qint64 bytesAvailable() const override;
        bool atEnd() const override;
        bool open(OpenMode mode) override;
        qint64 size() const override;
    protected:
        qint64 readData(char *data, qint64 maxlen) override;

        qint64 writeData(const char *data, qint64 len) override;

    private:
        mutable QMutex m_mutex;
        QWaitCondition m_dataAvailable;
        std::vector<char> m_buffer;
        size_t m_head = 0;
        size_t m_tail = 0;
        size_t m_size = 0;
        static constexpr size_t MAX_BUFFER_SIZE = 4 * 1024 * 1024; // 4 MB limit
    };
}
