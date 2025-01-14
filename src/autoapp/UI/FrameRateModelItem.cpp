#include <f1x/openauto/autoapp/UI/FrameRateModelItem.hpp>

namespace f1x::openauto::autoapp::UI {

  FrameRateModelItem::FrameRateModelItem(QString display,
                                         aap_protobuf::service::media::sink::message::VideoFrameRateType value,
                                         QObject *parent) :
      QObject(parent),
      m_display(display),
      m_value(value) {

  }

  QString FrameRateModelItem::getDisplay() const { return m_display; }

  void FrameRateModelItem::setDisplay(const QString &display) {
    if (m_display != display) {
      m_display = display;

    }
  }

  aap_protobuf::service::media::sink::message::VideoFrameRateType FrameRateModelItem::getValue() const {
    return m_value;
  }


  void FrameRateModelItem::setValue(const aap_protobuf::service::media::sink::message::VideoFrameRateType &value) {
    if (m_value != value) { // Extract T from QVariant
      m_value = value;

    }
  }
}