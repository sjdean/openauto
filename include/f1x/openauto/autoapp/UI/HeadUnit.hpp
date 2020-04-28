//
// Created by sjdea on 27/04/2020.
//

#ifndef OPENAUTO_HEADUNIT_H
#define OPENAUTO_HEADUNIT_H

#include <QObject>
#include <QString>

namespace f1x {
    namespace openauto {
        namespace autoapp {
            namespace ui {

                class HeadUnit : public QObject {
                    Q_OBJECT
                    Q_PROPERTY(QString username READ userName WRITE setUserName NOTIFY userNameChanged);

                public

                    explicit HeadUnit(QObject *parent = nullptr);

                    QString userName();

                    void setUserName(const QString &userName);

                    signals:
                            void

                    userNameChanged();

                private:
                    QString m_userName;
                };
            }
        }
    }
}

#endif //OPENAUTO_HEADUNIT_H
