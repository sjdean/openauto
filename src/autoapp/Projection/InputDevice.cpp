#include <f1x/openauto/autoapp/Projection/IInputDeviceEventHandler.hpp>
#include <f1x/openauto/autoapp/Projection/InputDevice.hpp>
#include <qloggingcategory.h>
#include <service/control/message/AudioFocusNotification.pb.h>
#include <service/inputsource/message/KeyEvent.pb.h>
#include <service/media/shared/message/Setup.pb.h>
#include <shared/MessageStatus.pb.h>
Q_LOGGING_CATEGORY(lcInput, "journeyos.input")

namespace f1x::openauto::autoapp::projection {
    InputDevice::InputDevice(QObject &parent, configuration::IConfiguration::Pointer configuration,
                           const QRect &touchscreenGeometry, const QRect &displayGeometry)
      : parent_(parent), configuration_(std::move(configuration)), touchscreenGeometry_(touchscreenGeometry),
        displayGeometry_(displayGeometry), eventHandler_(nullptr) {
    this->moveToThread(parent.thread());
  }

  void InputDevice::start(IInputDeviceEventHandler &eventHandler) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    qInfo(lcInput) << "[InputDevice] start()";
    eventHandler_ = &eventHandler;
    parent_.installEventFilter(this);
  }

  void InputDevice::stop() {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    qInfo(lcInput) << "[InputDevice] stop()";
    parent_.removeEventFilter(this);
    eventHandler_ = nullptr;
  }

  bool InputDevice::eventFilter(QObject *obj, QEvent *event) {
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if (eventHandler_ != nullptr) {
      if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (!key->isAutoRepeat()) {
          return this->handleKeyEvent(event, key);
        }
      } else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
                 event->type() == QEvent::MouseMove) {
        return this->handleTouchEvent(event);
      }
    }

    return QObject::eventFilter(obj, event);
  }

  bool InputDevice::handleKeyEvent(QEvent *event, QKeyEvent *key) {
    auto eventType = event->type() == QEvent::KeyPress ? ButtonEventType::PRESS : ButtonEventType::RELEASE;
    aap_protobuf::service::media::sink::message::KeyCode buttonCode;
    WheelDirection wheelDirection = WheelDirection::NONE;

    switch (key->key()) {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_CENTER;
        break;

      case Qt::Key_Left:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_LEFT;
        break;

      case Qt::Key_Right:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_RIGHT;
        break;

      case Qt::Key_Up:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_UP;
        break;

      case Qt::Key_Down:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_DOWN;
        break;

      case Qt::Key_Escape:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_BACK;
        break;

      case Qt::Key_H:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_HOME;
        break;

      case Qt::Key_P:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_CALL;
        break;

      case Qt::Key_O:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ENDCALL;
        break;

      case Qt::Key_MediaPlay:
      case Qt::Key_X:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PLAY;
        break;

      case Qt::Key_MediaPause:
      case Qt::Key_C:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PAUSE;
        break;

      case Qt::Key_MediaPrevious:
      case Qt::Key_V:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PREVIOUS;
        break;

      case Qt::Key_MediaTogglePlayPause:
      case Qt::Key_B:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PLAY_PAUSE;
        break;

      case Qt::Key_MediaNext:
      case Qt::Key_N:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_NEXT;
        break;

      case Qt::Key_M:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_SEARCH;
        break;

      case Qt::Key_1:
        wheelDirection = WheelDirection::LEFT;
        eventType = ButtonEventType::NONE;
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER;
        break;

      case Qt::Key_2:
        wheelDirection = WheelDirection::RIGHT;
        eventType = ButtonEventType::NONE;
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER;
        break;

      case Qt::Key_F:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_NAVIGATION;
        break;

      default:
        return true;
    }

    const auto &buttonCodes = this->getSupportedButtonCodes();
    if (std::find(buttonCodes.begin(), buttonCodes.end(), buttonCode) != buttonCodes.end()) {
      if (buttonCode != aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER ||
          event->type() == QEvent::KeyRelease) {
        eventHandler_->onButtonEvent({eventType, wheelDirection, buttonCode});
      }
    }

    return true;
  }

  bool InputDevice::handleTouchEvent(QEvent *event) {
    if (!configuration_->hasTouchScreen()) {
      return true;
    }

    aap_protobuf::service::inputsource::message::PointerAction type;

    switch (event->type()) {
      case QEvent::MouseButtonPress:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_DOWN;
        break;
      case QEvent::MouseButtonRelease:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP;
        break;
      case QEvent::MouseMove:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_MOVED;
        break;
      default:
        return true;
    };

    QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
    if (event->type() == QEvent::MouseButtonRelease || mouse->buttons().testFlag(Qt::LeftButton)) {
      const uint32_t x =
          (static_cast<float>(mouse->pos().x()) / touchscreenGeometry_.width()) * displayGeometry_.width();
      const uint32_t y =
          (static_cast<float>(mouse->pos().y()) / touchscreenGeometry_.height()) * displayGeometry_.height();
      eventHandler_->onTouchEvent({type, x, y, 0});
    }

    return true;
  }

  bool InputDevice::hasTouchscreen() const {
    return configuration_->hasTouchScreen();
  }

  QRect InputDevice::getTouchscreenGeometry() const {
    return touchscreenGeometry_;
  }

  IInputDevice::ButtonCodes InputDevice::getSupportedButtonCodes() const {
      std::vector<aap_protobuf::service::media::sink::message::KeyCode> codes;
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_3D_MODE);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_PRIMARY_BUTTON);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_SECONDARY_BUTTON);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_TERTIARY_BUTTON);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_ROTARY_CONTROLLER);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_DPAD_CENTER);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_DPAD_LEFT);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_DPAD_RIGHT);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_DPAD_UP);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_DPAD_DOWN);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MEDIA_NEXT);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MEDIA_PREVIOUS);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MEDIA_PLAY);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MEDIA_PLAY_PAUSE);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MEDIA_STOP);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_BACK);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_FORWARD);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_BRIGHTNESS_UP);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_BRIGHTNESS_DOWN);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_VOLUME_UP);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_VOLUME_DOWN);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_VOLUME_MUTE);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_NAVIGATION);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_HOME);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MUSIC);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_RADIO);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_SEARCH);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_CALL);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_ENDCALL);
      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_TEL);

      codes.push_back(aap_protobuf::service::media::sink::message::KEYCODE_MUTE);

      return codes;
    }

}
