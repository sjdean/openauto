#pragma once

#include <QDBusMessage>
#include <QObject>
#include <QDBusObjectPath>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logAgent)

class BluetoothAgent : public QObject {
    Q_OBJECT

public:
    explicit BluetoothAgent(const QString &dbusPath, QObject *parent = nullptr);
    virtual ~BluetoothAgent();

    // Getter for the D-Bus path
    QString objectPath() const { return m_dbusPath; }

signals:
    /**
     * @brief Emitted when BlueZ requests numeric comparison.
     * @param passkey The 6-digit passkey to display (e.g., "456123").
     */
    void showConfirmation(const QString &passkey);

    /**
     * @brief Emitted when BlueZ requests a PIN code.
     * @param pincode The PIN code to display (e.g., "0000").
     */
    void showPinCode(const QString &pincode);

    /**
     * @brief Emitted when pairing is finished (succeeded or failed).
     * Your QML can use this to close the PIN popup.
     */
    void pairingComplete();

public slots:
    // --- D-Bus Slots (Called by BlueZ) ---

    /**
     * @brief D-Bus: BlueZ asks our agent to display a PIN.
     * We emit a signal to QML and send the PIN back to BlueZ.
     */
    QString RequestPinCode(const QDBusObjectPath &device);

    /**
     * @brief D-Bus: BlueZ asks for "Yes/No" confirmation of a passkey.
     * We store the D-Bus message and emit a signal to QML.
     */
    void RequestConfirmation(const QDBusMessage &message, const QDBusObjectPath &device, quint32 passkey);

    /**
     * @brief D-Bus: BlueZ tells our agent it's no longer needed.
     */
    void Release();

    /**
     * @brief D-Bus: BlueZ asks to authorize a service (e.g., phone book).
     * We'll just auto-accept this for now.
     */
    static void AuthorizeService(const QDBusObjectPath &device, const QString &uuid);

    /**
     * @brief D-Bus: BlueZ tells us the pairing was canceled.
     */
    void Cancel();


    // --- QML-Callable Slots (Called by your UI) ---

    /**
     * @brief Q_INVOKABLE: User clicked "Accept" in the QML popup.
     */
    Q_INVOKABLE void accept();

    /**
     * @brief Q_INVOKABLE: User clicked "Decline" in the QML popup.
     */
    Q_INVOKABLE void reject();

private:
    /**
     * @brief Helper to send the "Accept" or "Decline" reply to D-Bus.
     */
    void replyToMessage(bool confirmed);

    QString m_dbusPath;
    QDBusMessage m_pendingMessage; // Stores the D-Bus message while we wait for the user
};