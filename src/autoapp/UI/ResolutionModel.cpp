#include <f1x/openauto/autoapp/UI/ResolutionModel.hpp>

namespace f1x::openauto::autoapp::UI {
  ResolutionModel::ResolutionModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
    ResolutionModel::populateComboBoxItems();
  }

  void ResolutionModel::populateComboBoxItems() {
    m_comboBoxItems.clear();
    fprintf(stderr, "Populate Resolution Items\n");
    addComboBoxItem("800 x 480",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_800x480);
    addComboBoxItem("1280 x 720",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1280x720);
    addComboBoxItem("1920 x 1080",
                    aap_protobuf::service::media::sink::message::VideoCodecResolutionType::VIDEO_1920x1080);

  }

  QList<QObject *> ResolutionModel::getComboBoxItems() const {
    fprintf(stderr, "Get Resolution Items\n");
    QList<QObject *> list;
    for (ResolutionModelItem *item: m_comboBoxItems) {
      fprintf(stderr, "Add Resolution Item\n");
      list.append(item);
    }
    fprintf(stderr, "Return Resolution Items\n");
    return list;
  }

  ResolutionModelItem* ResolutionModel::getCurrentComboBoxItem() {
    fprintf(stderr, "Get Resolution Item\n");
    if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
      fprintf(stderr, "Empty or not set\n");
      m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
    }
    fprintf(stderr, "Return Resolution Items\n");
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