#ifndef OPENAUTO_CANBUSPORTINFOITEM_HPP
#define OPENAUTO_CANBUSPORTINFOITEM_HPP

#include <QObject>
#include <QString>

namespace f1x::openauto::autoapp::UI::Model::List {

// Read-only snapshot of a single physical CAN port as reported in a
// DeviceAnnounce.  Populated from the QVariantMap entries in the
// DeviceManager::deviceAnnounced signal.

class CanBusPortInfoItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString portId     READ getPortId     CONSTANT)
    Q_PROPERTY(quint32 bitrate    READ getBitrate    CONSTANT)
    Q_PROPERTY(bool    fdCapable  READ isFdCapable   CONSTANT)
    Q_PROPERTY(bool    linkActive READ isLinkActive  CONSTANT)

public:
    CanBusPortInfoItem(const QString& portId,
                       quint32        bitrate,
                       bool           fdCapable,
                       bool           linkActive,
                       QObject*       parent = nullptr)
        : QObject(parent)
        , m_portId(portId)
        , m_bitrate(bitrate)
        , m_fdCapable(fdCapable)
        , m_linkActive(linkActive)
    {}

    QString getPortId()    const { return m_portId; }
    quint32 getBitrate()   const { return m_bitrate; }
    bool    isFdCapable()  const { return m_fdCapable; }
    bool    isLinkActive() const { return m_linkActive; }

private:
    QString m_portId;
    quint32 m_bitrate;
    bool    m_fdCapable;
    bool    m_linkActive;
};

} // namespace f1x::openauto::autoapp::UI::Model::List

#endif // OPENAUTO_CANBUSPORTINFOITEM_HPP