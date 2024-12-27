//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/FrameRateModel.hpp>

FrameRateModel::FrameRateModel(QObject *parent) : ComboBoxModel(parent) {
    FrameRateModel::populateComboBoxItems();
}

void FrameRateModel::populateComboBoxItems() {
  addComboBoxItem("60 FPS",
               aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_60);  // VIDEO_FPS_60 from ProtoBuf
  addComboBoxItem("30 FPS",
               aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_30);  // VIDEO_FPS_30 from ProtoBuf
}
