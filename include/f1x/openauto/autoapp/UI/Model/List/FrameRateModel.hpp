#ifndef OPENAUTO_FRAMERATEMODEL_HPP
#define OPENAUTO_FRAMERATEMODEL_HPP

#include <QList>
#include <QQmlListProperty>
#include <QtCore/QObject>
#include <f1x/openauto/autoapp/UI/Model/List/FrameRateModelItem.hpp>

namespace f1x::openauto::autoapp::UI::Model::List {
  class FrameRateModel : public QObject {
  Q_OBJECT
    Q_PROPERTY(QList<QObject *> comboBoxItems READ getComboBoxItems NOTIFY comboBoxItemsChanged)
    Q_PROPERTY(FrameRateModelItem* currentComboBoxItem READ getCurrentComboBoxItem WRITE setCurrentComboBoxItem NOTIFY currentComboBoxItemChanged)

  public:
    explicit FrameRateModel(QObject *parent = nullptr);

  signals:
    void comboBoxItemsChanged();
    void currentComboBoxItemChanged();

  protected:
    QList<QObject *> getComboBoxItems() const;
    FrameRateModelItem* getCurrentComboBoxItem();
    void setCurrentComboBoxItem(FrameRateModelItem* value);

    void populateComboBoxItems();
    void addComboBoxItem(const QString &display, aap_protobuf::service::media::sink::message::VideoFrameRateType value);
  private:
    QList<FrameRateModelItem *> m_comboBoxItems;
    FrameRateModelItem* m_currentComboBoxItem;
  };
}
#endif //OPENAUTO_FRAMERATEMODEL_HPP
