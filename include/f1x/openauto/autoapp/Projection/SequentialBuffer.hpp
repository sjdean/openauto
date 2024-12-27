

#pragma once#include <QIODevice>
#include <mutex>
#include <boost/circular_buffer.hpp>
#include <aasdk/Common/Data.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class SequentialBuffer: public QIODevice
{
public:
    SequentialBuffer();
    bool isSequential() const override;
    qint64 size() const override;
    qint64 pos() const override;
    bool seek(qint64 pos) override;
    bool atEnd() const override;
    bool reset() override;
    bool canReadLine() const override;
    qint64 bytesAvailable() const override;
    bool open(OpenMode mode) override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    boost::circular_buffer<aasdk::common::Data::value_type> data_;
    mutable std::mutex mutex_;
};

}
}
}
}
