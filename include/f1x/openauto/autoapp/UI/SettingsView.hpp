#ifndef OPENAUTO_SETTINGSVIEW_H
#define OPENAUTO_SETTINGSVIEW_H

#include <QtCore/QObject>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

class SettingsView : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString carMake READ carMake WRITE setCarMake NOTIFY carMakeChanged)
  Q_PROPERTY(QString carModel READ carModel WRITE setCarModel NOTIFY carModelChanged)
  Q_PROPERTY(bool autoPlayback READ autoPlayback WRITE setAutoPlayback NOTIFY autoPlaybackChanged)
  Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
  Q_PROPERTY(bool channelMedia READ channelMedia WRITE setChannelMedia NOTIFY channelMediaChanged)
  Q_PROPERTY(bool channelGuidance READ channelGuidance WRITE setChannelGuidance NOTIFY channelGuidanceChanged)
  Q_PROPERTY(bool channelTelephony READ channelTelephony WRITE setChannelTelephony NOTIFY channelTelephonyChanged)
  Q_PROPERTY(bool audioRt READ audioRt WRITE setAudioRt NOTIFY audioRtChanged)
  Q_PROPERTY(bool audioQt READ audioQt WRITE setAudioQt NOTIFY audioQtChanged)
  Q_PROPERTY(int omxLayer READ omxLayer WRITE setOmxLayer NOTIFY omxLayerChanged)
  Q_PROPERTY(int marginHeight READ marginHeight WRITE setMarginHeight NOTIFY marginHeightChanged)
  Q_PROPERTY(int marginWidth READ marginWidth WRITE setMarginWidth NOTIFY marginWidthChanged)
  Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)
  Q_PROPERTY(int volumePlayback READ volumePlayback WRITE setVolumePlayback NOTIFY volumePlaybackChanged)
  Q_PROPERTY(int volumeCapture READ volumeCapture WRITE setVolumeCapture NOTIFY volumeCaptureChanged)
  Q_PROPERTY(int brightnessDayMin READ brightnessDayMin WRITE setBrightnessDayMin NOTIFY brightnessDayMinChanged)
  Q_PROPERTY(int brightnessDayMax READ brightnessDayMax WRITE setBrightnessDayMax NOTIFY brightnessDayMaxChanged)
  Q_PROPERTY(int brightnessNightMin READ brightnessNightMin WRITE seBbrightnessNightMin NOTIFY brightnessNightMinChanged)
  Q_PROPERTY(int brightnessNightMax READ brightnessNightMax WRITE setBrightnessNightMax NOTIFY brightnessNightMaxChanged)
  Q_PROPERTY(bool videoEgl READ videoEgl WRITE setVideoEgl NOTIFY videoEglChanged)
  Q_PROPERTY(bool videoX11 READ videoX11 WRITE setVideoX11 NOTIFY videoX11Changed)
  Q_PROPERTY(bool rotateDisplay READ rotateDisplay WRITE setRotateDisplay NOTIFY rotateDisplayChanged)

public:
  explicit SettingsView(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, QObject *parent = nullptr);

signals:

  void carMakeChanged();
  void carModelChanged();

  void autoPlaybackChanged();
  void autoStartChanged();
  void channelMediaChanged();
  void channelGuidanceChanged();
  void channelTelephonyChanged();
  void omxLayerChanged();
  void marginHeightChanged();
  void marginWidthChanged();
  void dpiChanged();

  void audioQtChanged();
  void audioRtChanged();

  void volumePlaybackChanged();
  void volumeCaptureChanged();

  void rotateDisplayChanged();

  void brightnessDayMinChanged();
  void brightnessDayMaxChanged();
  void brightnessNightMinChanged();
  void brightnessNightMaxChanged();

private:
  f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration_;

  // Car Settings
  QString m_carMake;
  QString m_carModel;

  // Media Settings
  bool m_autoPlayback;
  bool m_autoStart;

  // Android Auto Settings - Channels
  bool m_channelMedia;
  bool m_channelGuidance;
  bool m_channelTelephony;

  // Android Auto Settings - Display
  int m_omxLayer;
  int m_marginHeight;
  int m_marginWidth;
  int m_dpi;

  // Audio Settings
  f1x::openauto::autoapp::configuration::AudioOutputBackendType m_audioType;
  int m_volumePlayback;
  int m_volumeCapture;

  // Video Settings
  int m_videoType;
  bool m_rotateDisplay;
  int m_brightnessDayMin;
  int m_brightnessDayMax;
  int m_brightnessNightMin;
  int m_brightnessNightMax;

  void setDpi(int value);

  int dpi();

  void setRotateDisplay(bool value);

  bool rotateDisplay();

  bool videoX11();

  bool videoEgl();

  void setBrightnessNightMax(int value);

  void setBrightnessNightMin(int value);

  int brightnessNightMax();

  int brightnessNightMin();

  int brightnessDayMin();

  int brightnessDayMax();

  void setBrightnessDayMin(int value);
  void setBrightnessDayMax(int value);

  void setVolumeCapture(int value);

  int volumeCapture();

  void setVolumePlayback(int value);

  int volumePlayback();

  void setMarginWidth(int value);

  int marginWidth();

  void setMarginHeight(int value);

  int marginHeight();

  void setOmxLayer(int value);

  int omxLayer();

  void setAudioQt(bool value);

  bool audioQt();

  void setAudioRt(bool value);

  bool audioRt();

  bool channelTelephony();

  void setChannelTelephony(bool value);

  void setChannelGuidance(bool value);

  bool channelGuidance();

  void setChannelMedia(bool value);

  bool channelMedia();

  void setAutoStart(bool value);

  bool autoStart();

  void setAutoPlayback(bool value);

  bool autoPlayback();

  void setCarModel(QString value);

  QString carModel() const;

  void setCarMake(QString value);

  QString carMake() const;
};

#endif //OPENAUTO_SETTINGSVIEW_H
