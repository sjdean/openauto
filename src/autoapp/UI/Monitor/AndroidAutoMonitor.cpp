#include "f1x/openauto/autoapp/UI/Monitor/AndroidAutoMonitor.hpp"

namespace f1x::openauto::autoapp::UI::Monitor {
    /**
     * Monitor AndroidAuto Connectivity Status and emit signals when method or status is updated
     * @param parent
     */
    AndroidAutoMonitor::AndroidAutoMonitor(QObject *parent) : QObject(parent),
                                                              m_connectivityMethod(
                                                                  Enum::AndroidAutoConnectivityMethod::AA_INDETERMINATE),
                                                              m_connectivityState(
                                                                  Enum::AndroidAutoConnectivityState::AA_DISCONNECTED) {
    }

    /**
     * Updates the Android Auto connectivity method.
     * @param method The new connectivity method to be updated.
     */
    void AndroidAutoMonitor::onConnectionMethodUpdate(const Enum::AndroidAutoConnectivityMethod::Value method) {
        if (m_connectivityMethod != method) {
            m_connectivityMethod = method;
            emit connectivityMethodChanged(method);
        }
    }

    /**
     * Updates the Android Auto connectivity state.
     * @param state The new connectivity state to be updated.
     */
    void AndroidAutoMonitor::onConnectionStateUpdate(const Enum::AndroidAutoConnectivityState::Value state) {
        if (m_connectivityState != state) {
            m_connectivityState = state;
            emit connectivityStateChanged(state);
        }
    }

    /**
     * Retrieves the current Android Auto connectivity state.
     * @return The current connectivity state of type Enum::AndroidAutoConnectivityState::Value.
     */
    Enum::AndroidAutoConnectivityState::Value AndroidAutoMonitor::getState() const {
        return m_connectivityState;
    }

    /**
     * Retrieves the current Android Auto connectivity method.
     * @return The current connectivity method of type Enum::AndroidAutoConnectivityMethod::Value.
     */
    Enum::AndroidAutoConnectivityMethod::Value AndroidAutoMonitor::getMethod() const {
        return m_connectivityMethod;
    }

    /**
     * Sets the new Android Auto connectivity state.
     * @param value The new value for the connectivity state of type Enum::AndroidAutoConnectivityState::Value.
     */
    void AndroidAutoMonitor::setState(const Enum::AndroidAutoConnectivityState::Value value) {
        if (m_connectivityState != value) {
            m_connectivityState = value;
            emit connectivityStateChanged(value);
        }
    }

    /**
     * Sets the Android Auto connectivity method.
     * @param value The new connectivity method of type Enum::AndroidAutoConnectivityMethod::Value.
     */
    void AndroidAutoMonitor::setMethod(const Enum::AndroidAutoConnectivityMethod::Value value) {
        if (m_connectivityMethod != value) {
            m_connectivityMethod = value;
            emit connectivityMethodChanged(value);
        }
    }
}
