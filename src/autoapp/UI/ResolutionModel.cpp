//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/ResolutionModel.hpp>

ResolutionModel::ResolutionModel(QObject *parent) : ComboBoxModel(parent) {
  ResolutionModel::populateComboBoxItems();


void ResolutionModel::populateComboBoxItems() {
  addComboBoxItem("800 x 480",
               aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_800x480);
  addComboBoxItem("1280 x 720",
               aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1280x720);
  addComboBoxItem("1920 x 1080",
                  aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1920x1080);

}