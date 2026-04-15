#pragma once
#include <f1x/openauto/autoapp/Service/MediaSink/VideoMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>


namespace f1x::openauto::autoapp::service::mediasink {
    class VideoService : public VideoMediaSinkService {
    public:
        VideoService(aasdk::messenger::IMessenger::Pointer messenger,
                     projection::IVideoOutput::Pointer videoOutput);

    protected:
        projection::IVideoOutput::Pointer videoOutput;
    };
}
