#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QAbstractListModel>

namespace f1x {
    namespace openauto {
        namespace autoapp {
            namespace ui {
                class SettingsModel : public QAbstractListModel {
                    Q_OBJECT

                public:
                    enum SettingsRole {
                        HeadingNameRole = Qt::DisplayRole,
                        OptionTypeRole
                    };
                    Q_ENUM(SettingsRole)

                    SettingsModel(QObject *parent = nullptr);

                    int rowCount(const QModelIndex & = QModelIndex()) const;
                    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
                    QHash<int, QByteArray> roleNames() const;

                    Q_INVOKABLE QVariantMap get(int row) const;
                    Q_INVOKABLE void append(const QString &headingName, const QString &optionType);
                    Q_INVOKABLE void set(int row, const QString &headingName, const QString &optionType);
                    Q_INVOKABLE void remove(int row);

                private:

                    struct Setting {
                        QString headingName;
                        QString optionType;
                    };


                    QList <Setting> m_settings;
                };
            }
        }
    }
}
#endif // CONTACTMODEL_H