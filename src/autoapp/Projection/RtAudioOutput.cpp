#include <f1x/openauto/autoapp/Projection/RtAudioOutput.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x::openauto::autoapp::projection {

  RtAudioOutput::RtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate)
      : channelCount_(channelCount), sampleSize_(sampleSize), sampleRate_(sampleRate) {
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

#if RTAUDIO_VERSION_MAJOR >= 6
    auto api = std::find(apis.begin(), apis.end(), RtAudio::LINUX_PULSE) != apis.end() ? RtAudio::LINUX_PULSE
                                                                                       : RtAudio::UNSPECIFIED;
    dac_ = std::make_unique<RtAudio>(api);
    dac_->setErrorCallback([](RtAudioErrorType type, const std::string &errorText) {
      switch (type) {
        case RTAUDIO_WARNING:
          OPENAUTO_LOG(warning) << "[RtAudioOutput] Warning: " << errorText;
          break;
        case RTAUDIO_NO_DEVICES_FOUND:
        case RTAUDIO_INVALID_DEVICE:
        case RTAUDIO_MEMORY_ERROR:
        case RTAUDIO_INVALID_PARAMETER:
        case RTAUDIO_DRIVER_ERROR:
        case RTAUDIO_SYSTEM_ERROR:
        case RTAUDIO_THREAD_ERROR:
        case RTAUDIO_NO_ERROR:
        case RTAUDIO_UNKNOWN_ERROR:
        case RTAUDIO_DEVICE_DISCONNECT:
        case RTAUDIO_INVALID_USE:
          OPENAUTO_LOG(error) << "[RtAudioOutput] Error: " << errorText;
          break;
      }
    });
#else
    dac_ = std::find(apis.begin(), apis.end(), RtAudio::LINUX_PULSE) == apis.end() ? std::make_unique<RtAudio>() : std::make_unique<RtAudio>(RtAudio::LINUX_PULSE);
#endif
  }

  bool RtAudioOutput::open() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (dac_->getDeviceCount() > 0) {
      RtAudio::StreamParameters parameters;
      parameters.deviceId = dac_->getDefaultOutputDevice();
      parameters.nChannels = channelCount_;
      parameters.firstChannel = 0;

#if RTAUDIO_VERSION_MAJOR >= 6

#else
      try {
#endif
      RtAudio::StreamOptions streamOptions;
      streamOptions.flags = RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_SCHEDULE_REALTIME;
      uint32_t bufferFrames = sampleRate_ == 16000 ? 1024 : 2048; //according to the observation of audio packets
      dac_->openStream(&parameters, nullptr, RTAUDIO_SINT16, sampleRate_, &bufferFrames,
                       &RtAudioOutput::audioBufferReadHandler, static_cast<void *>(this), &streamOptions);
      OPENAUTO_LOG(info) << "[RtAudioOutput] Sample Rate: " << sampleRate_;
      return audioBuffer_.open(QIODevice::ReadWrite);
#if RTAUDIO_VERSION_MAJOR >= 6

#else
      }
        catch(const RtAudioError& e)
        {
          // TODO: Later version of RtAudio uses a different mechanism - FIXME - support new versions
            OPENAUTO_LOG(error) << "[RtAudioOutput] Failed to open audio output, what: " << e.what();
        }
#endif
    } else {
      OPENAUTO_LOG(error) << "[RtAudioOutput] No output devices found.";
    }

    return false;
  }

  void
  RtAudioOutput::write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer &buffer) {
    audioBuffer_.write(reinterpret_cast<const char *>(buffer.cdata), buffer.size);
  }

  void RtAudioOutput::start() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (dac_->isStreamOpen() && !dac_->isStreamRunning()) {
#if RTAUDIO_VERSION_MAJOR >= 6

#else
      try {
#endif
      dac_->startStream();

#if RTAUDIO_VERSION_MAJOR >= 6

#else
      }
        catch(const RtAudioError& e)
        {
            OPENAUTO_LOG(error) << "[RtAudioOutput] Failed to start audio output, what: " << e.what();
        }
#endif
    }
  }

  void RtAudioOutput::stop() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    this->doSuspend();

    if (dac_->isStreamOpen()) {
      dac_->closeStream();
    }
  }

  void RtAudioOutput::suspend() {
    //not needed
  }

  uint32_t RtAudioOutput::getSampleSize() const {
    return sampleSize_;
  }

  uint32_t RtAudioOutput::getChannelCount() const {
    return channelCount_;
  }

  uint32_t RtAudioOutput::getSampleRate() const {
    return sampleRate_;
  }

  void RtAudioOutput::doSuspend() {
    if (dac_->isStreamOpen() && dac_->isStreamRunning()) {

#if RTAUDIO_VERSION_MAJOR >= 6

#else
      try {
#endif
      dac_->stopStream();

#if RTAUDIO_VERSION_MAJOR >= 6

#else
      }
            catch(const RtAudioError& e)
            {
                OPENAUTO_LOG(error) << "[RtAudioOutput] Failed to suspend audio output, what: " << e.what();
            }
#endif
    }
  }

  int RtAudioOutput::audioBufferReadHandler(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                            double streamTime, RtAudioStreamStatus status, void *userData) {
    RtAudioOutput *self = static_cast<RtAudioOutput *>(userData);
    std::lock_guard<decltype(self->mutex_)> lock(self->mutex_);

    const auto bufferSize = nBufferFrames * (self->sampleSize_ / 8) * self->channelCount_;
    self->audioBuffer_.read(reinterpret_cast<char *>(outputBuffer), bufferSize);
    return 0;
  }

}



