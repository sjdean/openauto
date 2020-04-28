#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QAbstractListModel>

class SettingsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum SettingsRole {
        FullNameRole = Qt::DisplayRole,
        AddressRole = Qt::UserRole,
        CityRole,
        NumberRole
    };
    Q_ENUM(SettingsRole)

    SettingsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE void append(const QString &fullName, const QString &address, const QString  &city, const QString &number);
    Q_INVOKABLE void set(int row, const QString &fullName, const QString &address, const QString  &city, const QString &number);
    Q_INVOKABLE void remove(int row);

private:
    struct Setting {
        QString headingName;
        QString optionType;
        QList<Option> options;
    };

    QList<Setting> m_settings;
};

#endif // CONTACTMODEL_H