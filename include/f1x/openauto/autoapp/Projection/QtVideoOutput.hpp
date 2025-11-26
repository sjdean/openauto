#pragma once
#include <QMediaPlayer>
#include <QVideoWidget>
#include <boost/noncopyable.hpp>
#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::projection {
    class QtVideoOutput : public QObject, public VideoOutput, boost::noncopyable {
        Q_OBJECT

    public:
        QtVideoOutput(configuration::IConfiguration::Pointer configuration);

        bool open() override;

        bool init() override;

        void write(uint64_t timestamp, const aasdk::common::DataConstBuffer &buffer) override;

        void stop() override;

    signals:
        void startPlayback();

        void stopPlayback();

    protected slots:
        void createVideoOutput();

        void onStartPlayback();

        void onStopPlayback() const;

    private:
        SequentialBuffer videoBuffer_;
        std::unique_ptr<QVideoWidget> videoWidget_;
        std::unique_ptr<QMediaPlayer> mediaPlayer_;
    };
}
