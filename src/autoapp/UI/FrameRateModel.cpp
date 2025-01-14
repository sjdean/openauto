#include <f1x/openauto/autoapp/UI/FrameRateModel.hpp>

namespace f1x::openauto::autoapp::UI {
  FrameRateModel::FrameRateModel(QObject *parent) : QObject(parent) {
    FrameRateModel::populateComboBoxItems();
  }

  void FrameRateModel::populateComboBoxItems() {
    addComboBoxItem("60 FPS",
                    aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_60);  // VIDEO_FPS_60 from ProtoBuf
    addComboBoxItem("30 FPS",
                    aap_protobuf::service::media::sink::message::VideoFrameRateType::VIDEO_FPS_30);  // VIDEO_FPS_30 from ProtoBuf
  }

  QList<QObject *> FrameRateModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (FrameRateModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  FrameRateModelItem* FrameRateModel::getCurrentComboBoxItem() const { return m_currentComboBoxItem; }

  void FrameRateModel::setCurrentComboBoxItem(FrameRateModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void FrameRateModel::addComboBoxItem(const QString &display,
                                       aap_protobuf::service::media::sink::message::VideoFrameRateType value) {
    auto item = FrameRateModelItem(display, value);
    m_comboBoxItems.emplace_back(&item);
  }
}