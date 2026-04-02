#include <f1x/openauto/autoapp/UI/Model/List/ResolutionModelItem.hpp>
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcComboResolutionItem, "journeyos.resolution.list.item")

namespace f1x::openauto::autoapp::UI::Model::List {
  ResolutionModelItem::ResolutionModelItem(QString display,
                                           aap_protobuf::service::media::sink::message::VideoCodecResolutionType value,
                                           QObject *parent) :
                                           QObject(parent),
                                           m_display(display),
                                           m_value(value) {

  }

  QString ResolutionModelItem::getDisplay() const { return m_display; }

  void ResolutionModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  aap_protobuf::service::media::sink::message::VideoCodecResolutionType ResolutionModelItem::getValue() const {
    return m_value;
  }

  void ResolutionModelItem::setValue(
      const aap_protobuf::service::media::sink::message::VideoCodecResolutionType &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}