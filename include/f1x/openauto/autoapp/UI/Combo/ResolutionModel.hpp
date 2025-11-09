#ifndef OPENAUTO_RESOLUTIONMODEL_HPP
#define OPENAUTO_RESOLUTIONMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Combo/ResolutionModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Combo {
  class ResolutionModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(ResolutionModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit ResolutionModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    ResolutionModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(ResolutionModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display,
                         aap_protobuf::service::media::sink::message::VideoCodecResolutionType value);
  private:
    QList<ResolutionModelItem *> m_comboBoxItems;
    ResolutionModelItem* m_currentComboBoxItem;
  };
}
#endif //OPENAUTO_RESOLUTIONMODEL_HPP
