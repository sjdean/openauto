#include <f1x/openauto/autoapp/UI/SettingsModel.hpp>

namespace f1x {
    namespace openauto {
        namespace autoapp {
            namespace ui {
                SettingsModel::SettingsModel(QObject *parent) : QAbstractListModel(parent) {
                    //OptionModel openModel = new OptionModel();
                    //optionModel.append("Left Handed", true);
                    //optionModel.append("Right Handed", false);


                    m_settings.append({"Handedness of Traffic", "R" });
                    m_settings.append("Enable Audio Channels", "C");
                    m_settings.append("Audio Output Type", "R");
                    m_settings.append("Screen DPI", "R");
                    m_settings.append("Screen Resolution", "R");
                    m_settings.append("Video FPS", "R");
                }

                int SettingsModel::rowCount(const QModelIndex &) const {
                    return m_settings.count();
                }

                QVariant SettingsModel::data(const QModelIndex &index, int role) const {
                    if (index.row() < rowCount())
                        switch (role) {
                            case HeadingNameRole:
                                return m_settings.at(index.row()).headingName;
                            case OptionTypeRole:
                                return m_settings.at(index.row()).optionType;
                            case OptionRole:
                                return m_settings.at(index.row()).options;
                            default:
                                return QVariant();
                        }
                    return QVariant();
                }

                QHash<int, QByteArray> ContactModel::roleNames() const {
                    static const QHash<int, QByteArray> roles{
                            {HeadingNameRole, "headingName"},
                            {OptionTypeRole,  "optionType"},
                            {OptionRole,     "options"}
                    };
                    return roles;
                }

                QVariantMap SettingsModel::get(int row) const {
                    const Setting setting = m_settings.value(row);
                    return {{"headingName", setting.headingName},
                            {"optionType",     setting.optionType},
                            {"options",   setting.options}};
                }

                void SettingsModel::append(const QString &headingName, const QString &optionType, const QList &options) {
                    int row = 0;
                    while (row < m_settings.count() && headingName > m_settings.at(row).fullName)
                        ++row;
                    beginInsertRows(QModelIndex(), row, row);
                    m_settings.insert(row, {headingName, optionType, options});
                    endInsertRows();
                }

                void SettingsModel::set(int row, const QString &headingName, const QString &optionType, const QList &options) {
                    if (row < 0 || row >= m_settings.count())
                        return;

                    m_settings.replace(row, {headingName, optionType, options});
                    dataChanged(index(row, 0), index(row, 0), {HeadingNameRole, OptionTypeRole, OptionsRole});
                }

                void SettingsModel::remove(int row) {
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