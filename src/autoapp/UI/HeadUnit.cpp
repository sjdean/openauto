//
// Created by sjdea on 27/04/2020.
//


#include <f1x/openauto/autoapp/UI/HeadUnit.hpp>

// https://doc.qt.io/qt-5/qtquick-codesamples.html

namespace f1x {
    namespace openauto {
        namespace autoapp {
            namespace ui {
                HeadUnit::HeadUnit(QObject *parent) : QObject(parent) {

                }

                QString HeadUnit::userName() {
                    return m_userName;
                }

                void HeadUnit::setUserName(const QString &userName) {
                    if (userName == m_userNmae)
                        return;

                    m_userName = userName;
                    emit userNameChanged();
                }
            }
        }
    }
}
