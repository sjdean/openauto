#include "f1x/openauto/autoapp/Projection/QtAudioInput.hpp"
#include <QGuiApplication>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QStandardPaths>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcQtAudioIn, "journeyos.projection.audio.input.qt")

namespace f1x::openauto::autoapp::projection {
using configuration::ConfigGroup;
using configuration::ConfigKey;

  QtAudioInput::QtAudioInput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate,
                             configuration::IConfiguration::Pointer config)
      : ioDevice_(nullptr),
        configuration_(std::move(config)) {

    qRegisterMetaType<IAudioInput::StartPromise::Pointer>("StartPromise::Pointer");

    // 1. Build requestedFormat_ — what AASDK expects from us
    requestedFormat_.setChannelCount(static_cast<int>(channelCount));
    requestedFormat_.setSampleRate(static_cast<int>(sampleRate));
    if (sampleSize == 8) {
      requestedFormat_.setSampleFormat(QAudioFormat::UInt8);
    } else if (sampleSize == 16) {
      requestedFormat_.setSampleFormat(QAudioFormat::Int16);
    } else if (sampleSize == 32) {
      requestedFormat_.setSampleFormat(QAudioFormat::Int32);
    } else {
      requestedFormat_.setSampleFormat(QAudioFormat::Int16);
      qCWarning(lcQtAudioIn) << "Unknown sample size:" << sampleSize << "defaulting to Int16";
    }

    // audioFormat_ starts equal to the requested format; createAudioInput() may switch it
    // to the device's preferred format for capture (we then resample).
    audioFormat_ = requestedFormat_;

    connect(this, &QtAudioInput::startRecording, this, &QtAudioInput::onStartRecording, Qt::QueuedConnection);
    connect(this, &QtAudioInput::stopRecording,  this, &QtAudioInput::onStopRecording,  Qt::QueuedConnection);

    createAudioInput();
  }

  QtAudioInput::~QtAudioInput() {
    if (swrCtx_) {
      swr_free(&swrCtx_);
    }
    if (filterGraph_) {
      avfilter_graph_free(&filterGraph_);
    }
  }

  // ── Helpers ──────────────────────────────────────────────────────────────────

  AVSampleFormat QtAudioInput::qAudioFmtToAv(QAudioFormat::SampleFormat fmt) {
    switch (fmt) {
      case QAudioFormat::UInt8: return AV_SAMPLE_FMT_U8;
      case QAudioFormat::Int16: return AV_SAMPLE_FMT_S16;
      case QAudioFormat::Int32: return AV_SAMPLE_FMT_S32;
      case QAudioFormat::Float: return AV_SAMPLE_FMT_FLT;
      default:                  return AV_SAMPLE_FMT_S16;
    }
  }

  bool QtAudioInput::initSwrContext() {
    if (swrCtx_) { swr_free(&swrCtx_); swrCtx_ = nullptr; }

    int ret = 0;
#if HAS_CH_LAYOUT
    AVChannelLayout inLayout, outLayout;
    av_channel_layout_default(&inLayout,  audioFormat_.channelCount());
    av_channel_layout_default(&outLayout, requestedFormat_.channelCount());

    ret = swr_alloc_set_opts2(&swrCtx_,
        &outLayout, qAudioFmtToAv(requestedFormat_.sampleFormat()), requestedFormat_.sampleRate(),
        &inLayout,  qAudioFmtToAv(audioFormat_.sampleFormat()),     audioFormat_.sampleRate(),
        0, nullptr);

    av_channel_layout_uninit(&inLayout);
    av_channel_layout_uninit(&outLayout);
#else
    uint64_t inLayout = av_get_default_channel_layout(audioFormat_.channelCount());
    uint64_t outLayout = av_get_default_channel_layout(requestedFormat_.channelCount());

    swrCtx_ = swr_alloc_set_opts(nullptr,
        outLayout, qAudioFmtToAv(requestedFormat_.sampleFormat()), requestedFormat_.sampleRate(),
        inLayout,  qAudioFmtToAv(audioFormat_.sampleFormat()),     audioFormat_.sampleRate(),
        0, nullptr);
    if (!swrCtx_) ret = AVERROR(ENOMEM);
#endif

    if (ret < 0 || !swrCtx_) {
      char errbuf[AV_ERROR_MAX_STRING_SIZE];
      av_strerror(ret, errbuf, sizeof(errbuf));
      qCWarning(lcQtAudioIn) << "[Mic] swr_alloc_set_opts2 failed:" << errbuf;
      return false;
    }

    ret = swr_init(swrCtx_);
    if (ret < 0) {
      char errbuf[AV_ERROR_MAX_STRING_SIZE];
      av_strerror(ret, errbuf, sizeof(errbuf));
      qCWarning(lcQtAudioIn) << "[Mic] swr_init failed:" << errbuf;
      swr_free(&swrCtx_);
      swrCtx_ = nullptr;
      return false;
    }

    qInfo(lcQtAudioIn) << "[Mic] SwrContext created:"
                       << audioFormat_.sampleRate()    << "Hz"
                       << audioFormat_.channelCount()  << "ch"
                       << audioFormat_.sampleFormat()
                       << " → "
                       << requestedFormat_.sampleRate()    << "Hz"
                       << requestedFormat_.channelCount()  << "ch"
                       << requestedFormat_.sampleFormat();
    return true;
  }

  bool QtAudioInput::initFilterGraph() {
    if (filterGraph_) {
        avfilter_graph_free(&filterGraph_);
        filterGraph_ = nullptr;
    }

    filterGraph_ = avfilter_graph_alloc();
    if (!filterGraph_) {
        qCCritical(lcQtAudioIn) << "[Mic] Unable to create filter graph";
        return false;
    }

    char layout_buf[64];

#if HAS_CH_LAYOUT
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, audioFormat_.channelCount());
    av_channel_layout_describe(&ch_layout, layout_buf, sizeof(layout_buf));
    av_channel_layout_uninit(&ch_layout);
#else
    uint64_t ch_layout = av_get_default_channel_layout(audioFormat_.channelCount());
    av_get_channel_layout_string(layout_buf, sizeof(layout_buf), audioFormat_.channelCount(), ch_layout);
#endif

    char args[512];
    snprintf(args, sizeof(args),
             "time_base=1/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
             audioFormat_.sampleRate(),
             audioFormat_.sampleRate(),
             av_get_sample_fmt_name(qAudioFmtToAv(audioFormat_.sampleFormat())),
             layout_buf);

    const AVFilter *buffersrc  = avfilter_get_by_name("abuffer");
    const AVFilter *buffersink = avfilter_get_by_name("abuffersink");

    int ret = avfilter_graph_create_filter(&buffersrcCtx_, buffersrc, "in",
                                           args, nullptr, filterGraph_);
    if (ret < 0) {
        qCCritical(lcQtAudioIn) << "[Mic] Cannot create buffer source";
        return false;
    }

    ret = avfilter_graph_create_filter(&buffersinkCtx_, buffersink, "out",
                                       nullptr, nullptr, filterGraph_);
    if (ret < 0) {
        qCCritical(lcQtAudioIn) << "[Mic] Cannot create buffer sink";
        return false;
    }

    // Configure the sink to output the same format as input, so we only do denoising here.
    // Resampling to AASDK format happens later in swr_convert if needed.
    // Or we could let the filter graph handle resampling too, but let's keep it simple: just denoise.
    // Actually, anlmdn might change format to planar float internally, so we should let the sink
    // accept what the filter gives, or force it back.
    // Let's try to force the sink to output the capture format to minimize changes downstream.
    // However, anlmdn usually outputs float planar.
    // If we force the sink to output our capture format (e.g. S16), the graph will auto-insert a resampler.
    // This is good.

    enum AVSampleFormat out_sample_fmts[] = { qAudioFmtToAv(audioFormat_.sampleFormat()), AV_SAMPLE_FMT_NONE };
    // int64_t out_channel_layouts[] = { (int64_t)av_get_default_channel_layout(audioFormat_.channelCount()), -1 };
    // int out_sample_rates[] = { audioFormat_.sampleRate(), -1 };

    ret = av_opt_set_int_list(buffersinkCtx_, "sample_fmts", out_sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
         qCCritical(lcQtAudioIn) << "[Mic] Cannot set output sample format";
         return false;
    }

    // We don't strictly need to force channel layout or rate if we just want denoising,
    // but ensuring consistency is safer.
    // For now, let's just set the sample format so downstream code (swr) gets what it expects.

    // Create the filter chain: abuffer -> anlmdn -> abuffersink
    // "anlmdn=s=5:p=0.002:r=0.002:m=11" - moderate settings
    // s: strength (default 0.00001 to 10, default 0.00001 is very weak? No, default is 0.00001 but typical usage is higher)
    // Wait, ffmpeg docs say: s default 0.00001.
    // Let's try a stronger default. "anlmdn=s=1.0"
    // Or "afftdn" (FFT based) might be lighter on CPU.
    // Let's try "afftdn" first as it's often effective for speech.
    // "afftdn=nr=12:nf=-25" (noise reduction 12dB, noise floor -25dB)
    // Let's stick to the user request: "software-based noise filter".
    // I'll use "afftdn" as it's generally good for speech.
    // Inputs -> "afftdn" -> Outputs

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();

    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrcCtx_;
    outputs->pad_idx    = 0;
    outputs->next       = nullptr;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersinkCtx_;
    inputs->pad_idx    = 0;
    inputs->next       = nullptr;

    // Using afftdn (FFT-based denoiser)
    // nr: noise reduction in dB (0.01 to 97). Default 12.
    // nf: noise floor in dB (-80 to -20). Default -50.
    // nt: noise type (w=white, v=vinyl, s=shellac, ...). Default w.
    const char *filter_descr = "afftdn=nr=20:nf=-40";

    ret = avfilter_graph_parse_ptr(filterGraph_, filter_descr, &inputs, &outputs, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCCritical(lcQtAudioIn) << "[Mic] avfilter_graph_parse_ptr failed:" << errbuf;
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    ret = avfilter_graph_config(filterGraph_, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCCritical(lcQtAudioIn) << "[Mic] avfilter_graph_config failed:" << errbuf;
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return false;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    qInfo(lcQtAudioIn) << "[Mic] Filter graph initialized:" << filter_descr;
    return true;
  }

  // ── WAV debug recording ───────────────────────────────────────────────────────

  void QtAudioInput::writeWavHeader(QFile &f, int sampleRate, int channels, int bitsPerSample, bool isFloat) {
    auto w16 = [&](uint16_t v) { f.write(reinterpret_cast<const char *>(&v), 2); };
    auto w32 = [&](uint32_t v) { f.write(reinterpret_cast<const char *>(&v), 4); };
    auto str = [&](const char *s) { f.write(s, 4); };

    uint16_t fmtTag    = isFloat ? 3 : 1;
    uint16_t blockAlign = static_cast<uint16_t>(channels * bitsPerSample / 8);
    uint32_t byteRate   = static_cast<uint32_t>(sampleRate * blockAlign);

    str("RIFF"); w32(0);                                       // RIFF chunk size — patched later
    str("WAVE");
    str("fmt "); w32(16);                                      // fmt chunk
    w16(fmtTag); w16(static_cast<uint16_t>(channels));
    w32(static_cast<uint32_t>(sampleRate)); w32(byteRate);
    w16(blockAlign); w16(static_cast<uint16_t>(bitsPerSample));
    str("data"); w32(0);                                       // data chunk size — patched later
  }

  void QtAudioInput::patchWavSizes(QFile &f, qint64 dataBytes) {
    uint32_t dataChunk = static_cast<uint32_t>(dataBytes);
    uint32_t riffChunk = static_cast<uint32_t>(dataBytes + 36);
    f.seek(4);  f.write(reinterpret_cast<const char *>(&riffChunk), 4);
    f.seek(40); f.write(reinterpret_cast<const char *>(&dataChunk), 4);
  }

  void QtAudioInput::openDebugFiles() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    rawWavFile_.setFileName(dir + "/journeyos_mic_raw.wav");
    if (rawWavFile_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      writeWavHeader(rawWavFile_,
          audioFormat_.sampleRate(), audioFormat_.channelCount(),
          audioFormat_.bytesPerSample() * 8,
          audioFormat_.sampleFormat() == QAudioFormat::Float);
      rawWavDataBytes_ = 0;
      qInfo(lcQtAudioIn) << "[Mic][Debug] Recording raw audio to:" << rawWavFile_.fileName();
    } else {
      qCWarning(lcQtAudioIn) << "[Mic][Debug] Could not open raw WAV file for writing";
    }

    resampledWavFile_.setFileName(dir + "/journeyos_mic_resampled.wav");
    if (resampledWavFile_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      writeWavHeader(resampledWavFile_,
          requestedFormat_.sampleRate(), requestedFormat_.channelCount(),
          requestedFormat_.bytesPerSample() * 8,
          requestedFormat_.sampleFormat() == QAudioFormat::Float);
      resampledWavDataBytes_ = 0;
      qInfo(lcQtAudioIn) << "[Mic][Debug] Recording resampled audio to:" << resampledWavFile_.fileName();
    } else {
      qCWarning(lcQtAudioIn) << "[Mic][Debug] Could not open resampled WAV file for writing";
    }
  }

  void QtAudioInput::closeDebugFiles() {
    if (rawWavFile_.isOpen()) {
      patchWavSizes(rawWavFile_, rawWavDataBytes_);
      rawWavFile_.close();
      qInfo(lcQtAudioIn) << "[Mic][Debug] Saved raw WAV:" << rawWavDataBytes_ << "bytes →" << rawWavFile_.fileName();
    }
    if (resampledWavFile_.isOpen()) {
      patchWavSizes(resampledWavFile_, resampledWavDataBytes_);
      resampledWavFile_.close();
      qInfo(lcQtAudioIn) << "[Mic][Debug] Saved resampled WAV:" << resampledWavDataBytes_ << "bytes →" << resampledWavFile_.fileName();
    }
  }

  // ── createAudioInput ─────────────────────────────────────────────────────────

  void QtAudioInput::createAudioInput() {
    qInfo(lcQtAudioIn) << "--- Audio Input Device Enumeration ---";

    const auto allDevices = QMediaDevices::audioInputs();
    qInfo(lcQtAudioIn) << "Available input devices:" << allDevices.size();
    for (const auto &d : allDevices) {
      const auto pref = d.preferredFormat();
      qInfo(lcQtAudioIn) << "  [INPUT]" << d.description()
                         << "| id:" << d.id()
                         << "| preferred:"
                         << pref.sampleRate() << "Hz"
                         << pref.channelCount() << "ch"
                         << pref.sampleFormat();
    }

    QAudioDevice device = QMediaDevices::defaultAudioInput();
    qInfo(lcQtAudioIn) << "Default input device:" << device.description()
                       << "| id:" << device.id();

    // Check config for a specific microphone name
    QString configDeviceName = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioCaptureDevice);
    if (!configDeviceName.isEmpty()) {
      bool found = false;
      for (const auto &d : allDevices) {
        if (d.description() == configDeviceName) {
          device = d;
          found = true;
          break;
        }
      }
      qInfo(lcQtAudioIn) << "Config requests device:" << configDeviceName
                         << (found ? "— FOUND" : "— NOT FOUND, using default");
    }

    qInfo(lcQtAudioIn) << "Selected input device:" << device.description();
    qInfo(lcQtAudioIn) << "AASDK requested format:"
                       << requestedFormat_.sampleRate()   << "Hz"
                       << requestedFormat_.channelCount() << "ch"
                       << requestedFormat_.sampleFormat();

    // Always capture at the device's preferred format. isFormatSupported() can
    // return true for formats that CoreAudio/ALSA won't actually stream at
    // (causing IdleState with 0 bytes). Preferred + software resample is reliable.
    const auto preferred = device.preferredFormat();
    audioFormat_ = preferred;

    if (preferred == requestedFormat_) {
      qInfo(lcQtAudioIn) << "Device preferred format matches AASDK request — no resampling needed";
    } else {
      qInfo(lcQtAudioIn) << "Capturing at device preferred format, will resample to AASDK request:"
                         << preferred.sampleRate()   << "Hz"
                         << preferred.channelCount() << "ch"
                         << preferred.sampleFormat();
    }

    audioInput_ = std::make_unique<QAudioSource>(device, audioFormat_);
    audioInput_->setBufferSize(32768);
    audioInput_->setVolume(0.6f);

    connect(audioInput_.get(), &QAudioSource::stateChanged,
            this, &QtAudioInput::onAudioStateChanged, Qt::QueuedConnection);

    qInfo(lcQtAudioIn) << "QAudioSource created. Capture format:"
                       << audioFormat_.sampleRate()   << "Hz"
                       << audioFormat_.channelCount() << "ch"
                       << audioFormat_.sampleFormat()
                       << "| Buffer size:" << audioInput_->bufferSize()
                       << "| State:" << audioInput_->state()
                       << "| Error:" << audioInput_->error();

    // Initialise resampler only when capture format differs from what AASDK wants.
    if (audioFormat_ != requestedFormat_) {
      initSwrContext();
    }

    // Initialize filter graph
    initFilterGraph();

    qInfo(lcQtAudioIn) << "--- End Audio Input Enumeration ---";
  }

  // ── IAudioInput interface ────────────────────────────────────────────────────

  bool QtAudioInput::open() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return true;
  }

  bool QtAudioInput::isActive() const {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    return ioDevice_ != nullptr;
  }

  void QtAudioInput::read(ReadPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (ioDevice_ == nullptr) {
      promise->reject();
    } else if (readPromise_ != nullptr) {
      qCWarning(lcQtAudioIn) << "Read request dropped: Previous read still pending.";
      promise->reject();
    } else {
      readPromise_ = std::move(promise);

      // readyRead is edge-triggered: if data is already buffered before read() is called,
      // no further signal fires and the promise would never resolve. Always schedule
      // onReadyRead via the event loop so it runs after we release the mutex.
      QMetaObject::invokeMethod(this, "onReadyRead", Qt::QueuedConnection);
    }
  }

  void QtAudioInput::start(IAudioInput::StartPromise::Pointer promise) {
    Q_EMIT startRecording(std::move(promise));
  }

  void QtAudioInput::stop() {
    Q_EMIT stopRecording();
  }

  // getSampleSize/ChannelCount/SampleRate return what AASDK expects, not what we capture.
  uint32_t QtAudioInput::getSampleSize() const {
    switch (requestedFormat_.sampleFormat()) {
      case QAudioFormat::UInt8: return 8;
      case QAudioFormat::Int16: return 16;
      case QAudioFormat::Int32: return 32;
      case QAudioFormat::Float: return 32;
      default: return 16;
    }
  }

  uint32_t QtAudioInput::getChannelCount() const {
    return static_cast<uint32_t>(requestedFormat_.channelCount());
  }

  uint32_t QtAudioInput::getSampleRate() const {
    return static_cast<uint32_t>(requestedFormat_.sampleRate());
  }

  // ── Slots ─────────────────────────────────────────────────────────────────────

  void QtAudioInput::onAudioStateChanged(QAudio::State state) {
    qInfo(lcQtAudioIn) << "[Mic] QAudioSource state changed to:" << state
                       << "error:" << audioInput_->error();
    if (state == QAudio::IdleState) {
      qCWarning(lcQtAudioIn) << "[Mic] IdleState — audio device open but no data flowing.";
#ifdef Q_OS_MACOS
      qCWarning(lcQtAudioIn) << "[Mic] macOS: verify microphone permission in "
                                "System Preferences > Privacy & Security > Microphone. "
                                "The Terminal (or this app) must be listed and enabled.";
#endif
    }
  }

  void QtAudioInput::onStartRecording(StartPromise::Pointer promise) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    qInfo(lcQtAudioIn) << "Microphone Start — state before:"
                       << audioInput_->state()
                       << "error:" << audioInput_->error();
    ioDevice_ = audioInput_->start();

    qInfo(lcQtAudioIn) << "Microphone start() returned ioDevice_:" << (ioDevice_ ? "non-null" : "NULL")
                       << "| state:" << audioInput_->state()
                       << "| error:" << audioInput_->error();

    if (ioDevice_ != nullptr) {
      connect(ioDevice_, &QIODevice::readyRead, this, &QtAudioInput::onReadyRead, Qt::QueuedConnection);

      // Polling fallback: Qt6's FFmpeg multimedia backend does not reliably emit
      // readyRead from the pull-mode QIODevice. Poll every 20 ms so audio flows
      // regardless of whether the signal fires.
      pollTimer_ = new QTimer(this);
      pollTimer_->setInterval(20);
      connect(pollTimer_, &QTimer::timeout, this, &QtAudioInput::onReadyRead);
      pollTimer_->start();

      // Open WAV debug files if enabled in config
      try {
        const QString dbgRec = configuration_->getSettingByName<QString>(ConfigGroup::Audio, ConfigKey::AudioDebugRecord);
        debugRecordEnabled_ = (dbgRec == "true" || dbgRec == "1");
      } catch (const std::runtime_error& e) {
        qInfo(lcQtAudioIn) << "DebugRecord setting not found, defaulting to false.";
        debugRecordEnabled_ = false;
      }
      if (debugRecordEnabled_) openDebugFiles();

      promise->resolve();
    } else {
      qCCritical(lcQtAudioIn) << "Failed to start Audio Input Device.";
      promise->reject();
    }
  }

  void QtAudioInput::onStopRecording() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    qInfo(lcQtAudioIn) << "Microphone Stop";

    if (pollTimer_) {
      pollTimer_->stop();
      pollTimer_->deleteLater();
      pollTimer_ = nullptr;
    }

    if (readPromise_ != nullptr) {
      readPromise_->reject();
      readPromise_.reset();
    }

    if (ioDevice_ != nullptr) {
      ioDevice_->disconnect(this);
      ioDevice_ = nullptr;
    }

    audioInput_->stop();

    // Flush any samples queued in the resampler so the next session starts clean.
    if (swrCtx_) {
      swr_convert(swrCtx_, nullptr, 0, nullptr, 0);
    }

    closeDebugFiles();
  }

  void QtAudioInput::onReadyRead() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    // No pending read request — leave data in the buffer for when read() is called.
    if (readPromise_ == nullptr) {
      return;
    }

    if (ioDevice_ == nullptr) {
      return;
    }

    qint64 available = ioDevice_->bytesAvailable();

    static int readyReadCount = 0;
    ++readyReadCount;
    if (readyReadCount <= 5 || readyReadCount % 50 == 0) {
      qDebug(lcQtAudioIn) << "[Mic] onReadyRead #" << readyReadCount
                          << "available=" << available << "bytes"
                          << "state=" << audioInput_->state();
    }

    if (available <= 0) {
      return;
    }

    qint64 toRead = std::min(available, cSampleSize);
    aasdk::common::Data rawData(static_cast<size_t>(toRead), 0);
    auto readSize = ioDevice_->read(reinterpret_cast<char *>(rawData.data()), toRead);

    if (readSize <= 0) {
      if (readSize == -1) {
        qCWarning(lcQtAudioIn) << "[Mic] read() returned -1 (error)";
        readPromise_->reject();
        readPromise_.reset();
      }
      // readSize == 0: leave promise pending for next poll
      return;
    }

    if (readSize < toRead) {
      rawData.resize(static_cast<size_t>(readSize));
    }

    // Write raw capture bytes to debug WAV
    if (rawWavFile_.isOpen()) {
      rawWavFile_.write(reinterpret_cast<const char *>(rawData.data()), static_cast<qint64>(rawData.size()));
      rawWavDataBytes_ += static_cast<qint64>(rawData.size());
    }

    // ── Apply Noise Reduction Filter ──
    aasdk::common::Data filteredData;
    if (filterGraph_ && buffersrcCtx_ && buffersinkCtx_) {
        // Create frame from raw data
        AVFrame *frame = av_frame_alloc();
        frame->nb_samples = static_cast<int>(rawData.size()) / audioFormat_.bytesPerFrame();
        frame->format = qAudioFmtToAv(audioFormat_.sampleFormat());
        frame->sample_rate = audioFormat_.sampleRate();

#if HAS_CH_LAYOUT
      av_channel_layout_default(&frame->ch_layout, audioFormat_.channelCount());
#else
      frame->channel_layout = av_get_default_channel_layout(audioFormat_.channelCount());
#endif

        if (av_frame_get_buffer(frame, 0) >= 0) {
            // Copy data to frame
            if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format))) {
                // Deinterleave if planar (unlikely for QAudioSource capture, but good to handle)
                // QAudioSource usually gives interleaved.
                // If it is planar, we need to split rawData.
                // Assuming interleaved for now as that's standard for QAudioSource.
                // If format is planar, we might have an issue here.
                // But qAudioFmtToAv maps to non-planar types (e.g. S16, not S16P) except for Float?
                // Wait, AV_SAMPLE_FMT_FLT is planar? No, FLT is interleaved, FLTP is planar.
                // My qAudioFmtToAv returns AV_SAMPLE_FMT_FLT which is interleaved. Good.
                memcpy(frame->data[0], rawData.data(), rawData.size());
            } else {
                memcpy(frame->data[0], rawData.data(), rawData.size());
            }

            // Push frame to filter
            if (av_buffersrc_add_frame_flags(buffersrcCtx_, frame, AV_BUFFERSRC_FLAG_KEEP_REF) >= 0) {
                // Pull filtered frames
                while (true) {
                    AVFrame *filtFrame = av_frame_alloc();
                    int ret = av_buffersink_get_frame(buffersinkCtx_, filtFrame);
                    if (ret < 0) {
                        av_frame_free(&filtFrame);
                        break;
                    }

                    // Append filtered data to output buffer
                    int bytesPerSample = av_get_bytes_per_sample(static_cast<AVSampleFormat>(filtFrame->format));
#if HAS_CH_LAYOUT
                  int channels = filtFrame->ch_layout.nb_channels;
#else
                  int channels = filtFrame->channels;
#endif
                    int size = filtFrame->nb_samples * channels * bytesPerSample;

                    size_t oldSize = filteredData.size();
                    filteredData.resize(oldSize + size);
                    memcpy(filteredData.data() + oldSize, filtFrame->data[0], size);

                    av_frame_free(&filtFrame);
                }
            }
        }
        av_frame_free(&frame);
    } else {
        // Fallback if filter not working
        filteredData = rawData;
    }

    // If filter produced no data (e.g. buffering), wait for next chunk
    if (filteredData.empty()) {
        return;
    }

    // Resample if capture format (or filter output) differs from what AASDK expects.
    if (swrCtx_) {
      int inBytesPerFrame  = audioFormat_.bytesPerFrame();
      int outBytesPerFrame = requestedFormat_.bytesPerFrame();

      int inSamples = static_cast<int>(filteredData.size()) / inBytesPerFrame;

      int64_t maxOutSamples = av_rescale_rnd(
          swr_get_delay(swrCtx_, audioFormat_.sampleRate()) + inSamples,
          requestedFormat_.sampleRate(), audioFormat_.sampleRate(), AV_ROUND_UP);

      aasdk::common::Data outData(static_cast<size_t>(maxOutSamples * outBytesPerFrame), 0);

      const uint8_t* inPtr  = filteredData.data();
            uint8_t* outPtr = outData.data();

      int converted = swr_convert(swrCtx_,
          &outPtr, static_cast<int>(maxOutSamples),
          &inPtr,  inSamples);

      if (converted > 0) {
        outData.resize(static_cast<size_t>(converted * outBytesPerFrame));
        // Write resampled bytes to debug WAV
        if (resampledWavFile_.isOpen()) {
          resampledWavFile_.write(reinterpret_cast<const char *>(outData.data()), static_cast<qint64>(outData.size()));
          resampledWavDataBytes_ += static_cast<qint64>(outData.size());
        }
        qDebug(lcQtAudioIn) << "[Mic] resampled" << readSize << "→" << outData.size() << "bytes, delivering to AASDK";
        readPromise_->resolve(std::move(outData));
      } else {
        qCWarning(lcQtAudioIn) << "[Mic] swr_convert returned" << converted << "— rejecting read";
        readPromise_->reject();
      }
    } else {
      // Write to resampled WAV too (same data since no resampling needed)
      if (resampledWavFile_.isOpen()) {
        resampledWavFile_.write(reinterpret_cast<const char *>(filteredData.data()), static_cast<qint64>(filteredData.size()));
        resampledWavDataBytes_ += static_cast<qint64>(filteredData.size());
      }
      qDebug(lcQtAudioIn) << "[Mic] delivering" << readSize << "bytes to AASDK";
      readPromise_->resolve(std::move(filteredData));
    }
    readPromise_.reset();
  }
}
