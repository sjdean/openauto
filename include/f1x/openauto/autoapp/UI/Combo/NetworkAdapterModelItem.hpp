#ifndef OPENAUTO_NETWORKADAPTERMODELITEM_HPP
#define OPENAUTO_NETWORKADAPTERMODELITEM_HPP

#include <QObject>
#include <QVariant>

namespace f1x::openauto::autoapp::UI::Combo {
    class NetworkAdapterModelItem : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString displayName READ getDisplayName CONSTANT)
        Q_PROPERTY(QString interfaceName READ getInterfaceName CONSTANT)
        Q_PROPERTY(QString address READ getAddress CONSTANT)

    public:
        NetworkAdapterModelItem(QString displayName, QString interfaceName, QString address, QObject *parent = nullptr)
            : QObject(parent), m_displayName(std::move(displayName)), m_interfaceName(std::move(interfaceName)), m_address(std::move(address)) {}

        QString getDisplayName() const { return m_displayName; }
        QString getInterfaceName() const { return m_interfaceName; }
        QString getAddress() const { return m_address; }

    private:
        QString m_displayName;
        QString m_interfaceName;
        QString m_address;
    };
}

#endif //OPENAUTO_NETWORKADAPTERMODELITEM_HPP