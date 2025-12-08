#include <f1x/openauto/autoapp/UI/Combo/ResolutionModel.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboResolution, "journeyos.resolution.list")

namespace f1x::openauto::autoapp::UI::Combo {
  ResolutionModel::ResolutionModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
    populateComboBoxItems();
  }

  void ResolutionModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    addComboBoxItem("800 x 480",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_800x480);
    addComboBoxItem("1280 x 720",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1280x720);
    addComboBoxItem("1920 x 1080",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1920x1080);

  }

  QList<QObject *> ResolutionModel::getComboBoxItems() const {
    QList<QObject *> list;
    for (ResolutionModelItem *item: m_comboBoxItems) {
      list.append(item);
    }
    return list;
  }

  ResolutionModelItem* ResolutionModel::getCurrentComboBoxItem() {
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    return m_currentComboBoxItem;
  }

  void ResolutionModel::setCurrentComboBoxItem(ResolutionModelItem* value) {
    if (m_currentComboBoxItem != value) {
      m_currentComboBoxItem = value;
      emit currentComboBoxItemChanged();
    }
  }

  void ResolutionModel::addComboBoxItem(const QString &display,
                                        aap_protobuf::service::media::sink::message::VideoCodecResolutionType value) {
    auto item = new ResolutionModelItem(display, value);
    m_comboBoxItems.emplace_back(item);
  }
}