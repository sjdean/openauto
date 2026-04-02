#include "f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityMethod.hpp"
#include "f1x/openauto/Common/Enum/AndroidAutoConnectivityState.hpp"
#include <qloggingcategory.h>
Q_LOGGING_CATEGORY(lcAA, "journeyos.aa.monitor")

namespace f1x::openauto::autoapp::UI::Monitor {
    /**
     * Monitor AndroidAuto Connectivity Status and emit signals when method or status is updated
     * @param parent
     */
    AndroidAutoMonitor::AndroidAutoMonitor(QObject *parent) : QObject(parent),
                                                              m_connectivityMethod(
                                                                  common::Enum::AndroidAutoConnectivityMethod::AA_INDETERMINATE),
                                                              m_connectivityState(
                                                                  common::Enum::AndroidAutoConnectivityState::AA_DISCONNECTED) {
    }

    /**
     * Updates the Android Auto connectivity method.
     * @param method The new connectivity method to be updated.
     */
    void AndroidAutoMonitor::onConnectionMethodUpdate(const common::Enum::AndroidAutoConnectivityMethod::Value method) {
        if (m_connectivityMethod != method) {
            m_connectivityMethod = method;
            emit connectivityMethodChanged(method);
        }
    }

    /**
     * Updates the Android Auto connectivity state.
     * @param state The new connectivity state to be updated.
     */
    void AndroidAutoMonitor::onConnectionStateUpdate(const common::Enum::AndroidAutoConnectivityState::Value state) {
        if (m_connectivityState != state) {
            m_connectivityState = state;
            emit connectivityStateChanged(state);
        }
    }

    /**
     * Retrieves the current Android Auto connectivity state.
     * @return The current connectivity state of type Enum::AndroidAutoConnectivityState::Value.
     */
    common::Enum::AndroidAutoConnectivityState::Value AndroidAutoMonitor::getState() const {
        return m_connectivityState;
    }

    /**
     * Retrieves the current Android Auto connectivity method.
     * @return The current connectivity method of type Enum::AndroidAutoConnectivityMethod::Value.
     */
    common::Enum::AndroidAutoConnectivityMethod::Value AndroidAutoMonitor::getMethod() const {
        return m_connectivityMethod;
    }

    /**
     * Sets the new Android Auto connectivity state.
     * @param value The new value for the connectivity state of type Enum::AndroidAutoConnectivityState::Value.
     */
    void AndroidAutoMonitor::setState(const common::Enum::AndroidAutoConnectivityState::Value value) {
        if (m_connectivityState != value) {
            m_connectivityState = value;
            emit connectivityStateChanged(value);
        }
    }

    /**
     * Sets the Android Auto connectivity method.
     * @param value The new connectivity method of type Enum::AndroidAutoConnectivityMethod::Value.
     */
    void AndroidAutoMonitor::setMethod(const common::Enum::AndroidAutoConnectivityMethod::Value value) {
        if (m_connectivityMethod != value) {
            m_connectivityMethod = value;
            emit connectivityMethodChanged(value);
        }
    }

    /**
     * Forwards a phone-supplied timestamp to any connected TimeController.
     * Safe to call from a non-Qt thread — the signal is queued to the main thread
     * automatically because TimeController lives on the main thread.
     */
    void AndroidAutoMonitor::notifyPhoneTimestamp(quint64 epochMicroseconds) {
        emit phoneTimestampReceived(epochMicroseconds);
    }
}
