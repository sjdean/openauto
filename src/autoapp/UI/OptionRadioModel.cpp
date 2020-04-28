#include <f1x/openauto/autoapp/UI/SettingsModel.hpp>

namespace f1x {
    namespace openauto {
        namespace autoapp {
            namespace ui {
                OptionRadioModel::OptionRadioModel(QObject *parent) : QAbstractListModel(parent) {
                    m_settings.append({"Left Hand Drive"}, 0);
                    m_settings.append({"Right Hand Drive"}, 1);
                }

                int OptionRadioModel::rowCount(const QModelIndex &) const {
                    return m_settings.count();
                }

                QVariant OptionRadioModel::data(const QModelIndex &index, int role) const {
                    if (index.row() < rowCount())
                        switch (role) {
                            case LabelRole:
                                return m_settings.at(index.row()).label;
                            case ValueRole:
                                return m_settings.at(index.row()).value;
                            default:
                                return QVariant();
                        }
                    return QVariant();
                }

                QHash<int, QByteArray> OptionRadioModel::roleNames() const {
                    static const QHash<int, QByteArray> roles{
                            {LabelRole, "label"},
                            {ValueRole,     "value"}
                    };
                    return roles;
                }

                QVariantMap OptionRadioModel::get(int row) const {
                    const Option option = m_settings.value(row);
                    return {{"label", option.label},
                            {"value",     option.value}
                    }

                    void OptionRadioModel::append(const QString &label, const bool &value) {
                        int row = 0;
                        while (row < m_settings.count() && label > m_settings.at(row).label)
                            ++row;
                        beginInsertRows(QModelIndex(), row, row);
                        m_settings.insert(row, {label,  value});
                        endInsertRows();
                    }

                    void OptionRadioModel::set(int row, const QString &label const bool &value) {
                        if (row < 0 || row >= m_settings.count())
                            return;

                        m_settings.replace(row, {label, value});
                        dataChanged(index(row, 0), index(row, 0), {LabelRole, ValueRole});
                    }

                    void OptionRadioModel::remove(int row) {
                        if (row < 0 || row >= m_settings.count())
                            return;

                        beginRemoveRows(QModelIndex(), row, row);
                        m_settings.removeAt(row);
                        endRemoveRows();
                    }
                }
            }
        }
    }