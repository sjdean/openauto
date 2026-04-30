#include "f1x/openauto/autoapp/UI/Combo/BluetoothAdapterModel.hpp"
#include <QBluetoothLocalDevice>
#include <QProcess>
#include <qloggingcategory.h>
#include <QDir>
#include <QFile>

Q_LOGGING_CATEGORY(lcComboBtAdapter, "journeyos.bluetooth.adapter.list")

// Resolve the BT SIG company identifier for the adapter that owns `address`.
// Reads /sys/class/bluetooth/hciN/manufacturer — an authoritative integer
// assigned by the Bluetooth SIG, not derived from the MAC OUI.
static QString btVendorName(const QBluetoothAddress &address)
{
#ifdef Q_OS_LINUX
    const QByteArray target = address.toString().toUpper().toLatin1();

    for (const QString &entry : QDir("/sys/class/bluetooth").entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (!entry.startsWith(QLatin1String("hci")))
            continue;

        const QString base = QStringLiteral("/sys/class/bluetooth/") + entry;

        QFile addrFile(base + QStringLiteral("/address"));
        if (!addrFile.open(QIODevice::ReadOnly))
            continue;
        if (addrFile.readAll().trimmed().toUpper() != target)
            continue;

        QFile mfFile(base + QStringLiteral("/manufacturer"));
        if (!mfFile.open(QIODevice::ReadOnly))
            break;

        bool ok = false;
        const int id = QString::fromLatin1(mfFile.readAll().trimmed()).toInt(&ok);
        if (!ok)
            break;

        // Bluetooth SIG Company Identifiers — https://www.bluetooth.com/specifications/assigned-numbers/
        static const QHash<int, QString> kCompany = {
            {  2,  "Intel" },
            {  9,  "Infineon" },
            { 10,  "CSR" },          // Cambridge Silicon Radio (now Qualcomm)
            { 13,  "Texas Instruments" },
            { 15,  "Broadcom" },
            { 18,  "Zeevo" },
            { 29,  "Qualcomm" },
            { 48,  "Qualcomm" },
            { 93,  "Intel" },
            { 308, "Intel" },
            { 320, "Murata" },
            { 1521,"Murata" },
            { 76,  "Apple" },
            { 224, "Google" },
            { 89,  "Nordic Semiconductor" },
            { 492, "Realtek" },
        };

        return kCompany.value(id, QString());
    }
#else
    Q_UNUSED(address)
#endif
    return QString();
}

namespace f1x::openauto::autoapp::UI::Combo {
    // TODO: Bring in Bluetooth Monitor
    BluetoothAdapterModel::BluetoothAdapterModel(QObject *parent) : QObject(parent), m_currentComboBoxItem(nullptr) {
        populateComboBoxItems();
    }

    void BluetoothAdapterModel::populateComboBoxItems() {
        m_comboBoxItems.clear();
        QList<QBluetoothHostInfo> adapters = QBluetoothLocalDevice::allDevices();

        if (!adapters.isEmpty()) {
            qDebug(lcComboBtAdapter) << "adapters found count=" << adapters.count();
            for (const QBluetoothHostInfo &adapter: adapters) {
                QString adapterAddress = adapter.address().toString();
                QString vendor = btVendorName(adapter.address());
                QString displayName = adapter.name().isEmpty() ? adapterAddress : adapter.name();

                QString displayString;
                if (vendor.isEmpty()) {
                    displayString = displayName + QStringLiteral(" — ") + adapterAddress;
                } else {
                    displayString = vendor + QStringLiteral(" (") + displayName + QStringLiteral(") — ") + adapterAddress;
                }

                addComboBoxItem(displayString.toUtf8().constData(), adapterAddress);
            }
        } else {
            qDebug(lcComboBtAdapter) << "no adapters found";
            addComboBoxItem("SettingsWindow", "none");
        }
    }

    QList<QObject *> BluetoothAdapterModel::getComboBoxItems() const {
        QList<QObject *> list;
        for (BluetoothAdapterModelItem *item: m_comboBoxItems) {
            list.append(item);
        }
        return list;
    }

    BluetoothAdapterModelItem *BluetoothAdapterModel::getCurrentComboBoxItem() {
        if (!m_currentComboBoxItem && !m_comboBoxItems.isEmpty()) {
            m_currentComboBoxItem = m_comboBoxItems.first(); // Select the first item by default
        }
        return m_currentComboBoxItem;
    }

    void BluetoothAdapterModel::setCurrentComboBoxItem(BluetoothAdapterModelItem *value) {
        if (m_currentComboBoxItem != value) {
            m_currentComboBoxItem = value;
            emit currentComboBoxItemChanged();
        }
    }

    void BluetoothAdapterModel::addComboBoxItem(const QString &display,
                                                QString value) {
        auto item = new BluetoothAdapterModelItem(display, value);
        m_comboBoxItems.emplace_back(item);
    }
}
