//
// Created by Simon Dean on 19/12/2024.
//#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/SettingsView.hpp>

SettingsView::SettingsView(f1x::openauto::autoapp::configuration::IConfiguration::Pointer configuration, QObject *parent) :
    QObject(parent),
    configuration_(std::move(configuration)),
    m_carMake(configuration_->getCarMake()),
    m_carModel(configuration_->getCarModel()),
    m_autoPlayback(configuration_->getAutoPlayback()),
    m_autoStart(configuration_->getAutoStart()),
    m_channelMedia(configuration_->getChannelMedia()),
    m_channelGuidance(configuration_->getChannelGuidance()),
    m_channelTelephony(configuration_->getChannelTelephony()),
    m_omxLayer(configuration_->getOMXLayer()),
    m_marginHeight(configuration_->getMargin().height()),
    m_marginWidth(configuration_->getMargin().width()),
    m_dpi(configuration_->getDPI()),
    m_audioType(configuration_->getAudioType()),
    m_volumePlayback(configuration_->getVolumePlaybackCurrent()),
    m_volumeCapture(configuration_->getVolumeCaptureCurrent()),
    m_videoType(configuration_->getVideoType()),
    m_rotateDisplay(configuration_->getRotateDisplay()),
    m_brightnessDayMin(configuration_->getBrightnessDayMin()),
    m_brightnessDayMax(configuration_->getBrightnessDayMax()),
    m_brightnessNightMin(configuration_->getBrightnessNightMin()),
    m_brightnessNightMax(configuration_->getBrightnessNightMax()) {

}

QString SettingsView::carMake() const {
  return m_carMake;
}

void SettingsView::setCarMake(QString value) {
  if (m_carMake != value) {
    m_carMake = value;
    emit carMakeChanged();
  }
}

QString SettingsView::carModel() const {
  return m_carModel;
}

void SettingsView::setCarModel(QString value) {
  if (m_carModel != value) {
    m_carModel = value;
    emit carModelChanged();
  }
}

bool SettingsView::autoPlayback() {
  return m_autoPlayback;
}

void SettingsView::setAutoPlayback(bool value) {
  if (m_autoPlayback != value) {
    m_autoPlayback = value;
    emit autoPlaybackChanged();
  }
}

bool SettingsView::autoStart() {
  return m_autoStart;
}

void SettingsView::setAutoStart(bool value) {
  if (m_autoStart != value) {
    m_autoStart = value;
    emit autoStartChanged();
  }
}

bool SettingsView::channelMedia() {
  return m_channelMedia;
}

void SettingsView::setChannelMedia(bool value) {
  if (m_channelMedia != value) {
    m_channelMedia = value;
    emit channelMediaChanged();
  }
}

bool SettingsView::channelGuidance() {
  return m_channelGuidance;
}

void SettingsView::setChannelGuidance(bool value) {
  if (m_channelGuidance != value) {
    m_channelGuidance = value;
    emit channelGuidanceChanged();
  }
}

bool SettingsView::channelTelephony() {
  return m_channelTelephony;
}

void SettingsView::setChannelTelephony(bool value) {
  if (m_channelTelephony != value) {
    m_channelTelephony = value;
    emit channelTelephonyChanged();
  }
}

bool SettingsView::audioRt() {
  return m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::RTAUDIO;
}

void SettingsView::setAudioRt(bool value) {
  // If Rt unselected and AudioType is Rt, or Rt selected and AudioType is Qt...
  if ((!value && m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::RTAUDIO) || (value && m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::QT)) {
    m_audioType = value ? f1x::openauto::autoapp::configuration::AudioOutputBackendType::RTAUDIO : f1x::openauto::autoapp::configuration::AudioOutputBackendType::QT;
    emit audioRtChanged();
    emit audioQtChanged();
  }
}

bool SettingsView::audioQt() {
  return m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::QT;
}

void SettingsView::setAudioQt(bool value) {
  // If Qt unselected and AudioType is Rt, or Qt selected and AudioType is Rt...
  if ((!value && m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::QT) || (value && m_audioType == f1x::openauto::autoapp::configuration::AudioOutputBackendType::RTAUDIO)) {
    m_audioType = value ? f1x::openauto::autoapp::configuration::AudioOutputBackendType::QT : f1x::openauto::autoapp::configuration::AudioOutputBackendType::RTAUDIO;
    emit audioRtChanged();
    emit audioQtChanged();
  }
}

int SettingsView::omxLayer() {
  return m_omxLayer;
}

void SettingsView::setOmxLayer(int value) {
  if (value != m_omxLayer) {
    m_omxLayer = value;
    emit omxLayerChanged();
  }
}

int SettingsView::marginHeight() {
  return m_marginHeight;
}

void SettingsView::setMarginHeight(int value) {
  if (value != m_marginHeight) {
    m_marginHeight = value;
    emit marginHeightChanged();
  }
}

int SettingsView::marginWidth() {
  return m_marginWidth;
}

void SettingsView::setMarginWidth(int value) {
  if (value != m_marginWidth) {
    m_marginWidth = value;
    emit marginWidthChanged();
  }
}

int SettingsView::volumePlayback() {
  return m_volumePlayback;
}

void SettingsView::setVolumePlayback(int value) {
  if (value != m_volumePlayback) {
    m_volumePlayback = value;
    emit volumePlaybackChanged();
  }
}

int SettingsView::volumeCapture() {
  return m_volumeCapture;
}

void SettingsView::setVolumeCapture(int value) {
  if (value != m_volumeCapture) {
    m_volumeCapture = value;
    emit volumeCaptureChanged();
  }
}

int SettingsView::brightnessDayMin() {
  return m_brightnessDayMin;
}

void SettingsView::setBrightnessDayMin(int value) {
  if (value != m_brightnessDayMin) {
    m_brightnessDayMin = value;
    emit brightnessDayMinChanged();
  }
}

int SettingsView::brightnessDayMax() {
  return m_brightnessDayMax;
}

void SettingsView::setBrightnessDayMax(int value) {
  if (value != m_brightnessDayMax) {
    m_brightnessDayMax = value;
    emit brightnessDayMaxChanged();
  }
}

int SettingsView::brightnessNightMin() {
  return m_brightnessNightMin;
}

void SettingsView::setBrightnessNightMin(int value) {
  if (value != m_brightnessNightMin) {
    m_brightnessNightMin = value;
    emit brightnessNightMinChanged();
  }
}

int SettingsView::brightnessNightMax() {
  return m_brightnessNightMax;
}

void SettingsView::setBrightnessNightMax(int value) {
  if (value != m_brightnessNightMax) {
    m_brightnessNightMax = value;
    emit brightnessNightMaxChanged();
  }
}

bool SettingsView::videoEgl() {
  return m_videoType == 1;
}

bool SettingsView::videoX11() {
  return m_videoType == 2;
}

bool SettingsView::rotateDisplay() {
  return m_rotateDisplay;
}

void SettingsView::setRotateDisplay(bool value) {
  if (m_rotateDisplay != value) {
    m_rotateDisplay = value;
    emit rotateDisplayChanged();
  }
}

int SettingsView::dpi() {
  return m_dpi;
}

void SettingsView::setDpi(int value) {
  if (value != m_dpi) {
    m_dpi = value;
    emit dpiChanged();
  }
}
