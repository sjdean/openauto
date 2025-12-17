#pragma once

#include <QObject>
#include <QKeyEvent>
#include <f1x/openauto/autoapp/Projection/IInputDevice.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::projection {
    class InputDevice : public QObject, public IInputDevice, boost::noncopyable {
        Q_OBJECT

    public:
        InputDevice(QObject &parent, configuration::IConfiguration::Pointer configuration,
                    const QRect &touchscreenGeometry,
                    const QRect &videoGeometry);

        void start(IInputDeviceEventHandler &eventHandler) override;

        void stop() override;

        ButtonCodes getSupportedButtonCodes() const override;

        bool eventFilter(QObject *obj, QEvent *event) override;

        bool hasTouchscreen() const override;

        QRect getTouchscreenGeometry() const override;

        Q_INVOKABLE bool handleTouchEvent(int type, float x, float y);
        Q_INVOKABLE bool handleMouseEvent(int type, float x, float y);

    private:
        void setVideoGeometry();

        bool handleKeyEvent(QEvent *event, QKeyEvent *key);

        void dispatchKeyEvent(ButtonEvent event);

        QObject &parent_;
        configuration::IConfiguration::Pointer configuration_;
        QRect touchscreenGeometry_;
        QRect displayGeometry_;
        IInputDeviceEventHandler *eventHandler_;
        std::mutex mutex_;
        bool hasTouchScreen_;
    };
}
