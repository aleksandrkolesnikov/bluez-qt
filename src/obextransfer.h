#ifndef QBLUEZ_OBEXTRANSFER_H
#define QBLUEZ_OBEXTRANSFER_H

#include <QObject>

#include "qbluez_export.h"

class QDBusObjectPath;

namespace QBluez
{

class PendingCall;
class ObexSession;

/**
 * OBEX transfer.
 *
 * This class represents transfer of one file.
 */
class QBLUEZ_EXPORT ObexTransfer : public QObject
{
    Q_OBJECT

    Q_ENUMS(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(ObexSession* session READ session)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(quint64 time READ time)
    Q_PROPERTY(quint64 size READ size)
    Q_PROPERTY(quint64 transferred READ transferred NOTIFY transferredChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool suspendable READ isSuspendable)

public:
    /**
     * Status of transfer.
     */
    enum Status {
        /** Indicates that the transfer is queued. */
        Queued,
        /** Indicates that the transfer is active. */
        Active,
        /** Indicates that the transfer is suspended. */
        Suspended,
        /** Indicates that the transfer have completed successfuly. */
        Complete,
        /** Indicates that the transfer have failed with error. */
        Error,
        /** Indicates that the transfer status is unknown. */
        Unknown
    };

    /**
     * Destroys an ObexTransfer object.
     */
    ~ObexTransfer();

    /**
     * D-Bus object path of the transfer.
     *
     * @return object path of transfer
     */
    QDBusObjectPath objectPath() const;

    /**
     * Returns the status of the transfer.
     *
     * @return status of transfer
     */
    Status status() const;

    /**
     * Returns the session of the transfer.
     *
     * @return session of transfer
     */
    ObexSession *session() const;

    /**
     * Returns the name of the transferred object.
     *
     * @return name of transferred object
     */
    QString name() const;

    /**
     * Returns the type of the transferred object.
     *
     * @return type of transferred object
     */
    QString type() const;

    /**
     * Returns the time of the transferred object.
     *
     * @return time of transferred object
     */
    quint64 time() const;

    /**
     * Returns the total size of the transferred object.
     *
     * @return size of transferred object
     */
    quint64 size() const;

    /**
     * Returns the number of bytes transferred.
     *
     * @return number of bytes transferred
     */
    quint64 transferred() const;

    /**
     * Returns the full name of the transferred file.
     *
     * @return full name of transferred file
     */
    QString fileName() const;

    /**
     * Returns whether the transfer is suspendable.
     *
     * @return true if transfer is suspendable
     */
    bool isSuspendable() const;

    /**
     * Stops the current transfer.
     *
     * Possible errors: PendingCall::NotAuthorized, PendingCall::InProgress
     *                  PendingCall::Failed
     *
     * @return void pending call
     */
    PendingCall *cancel();

    /**
     * Suspends the current transfer.
     *
     * Only suspendable transfers can be suspended.
     *
     * Possible errors: PendingCall::NotAuthorized, PendingCall::NotInProgress
     *
     * @see isSuspendable()
     *
     * @return void pending call
     */
    PendingCall *suspend();

    /**
     * Resumes the current transfer.
     *
     * Possible errors: PendingCall::NotAuthorized, PendingCall::NotInProgress
     *
     * @return void pending call
     */
    PendingCall *resume();

Q_SIGNALS:
    /**
     * Indicates that the status of transfer have changed.
     */
    void statusChanged(Status status);

    /**
     * Indicates that the number of transferred bytes have changed.
     */
    void transferredChanged(quint64 transferred);

    /**
     * Indicates that the name of transferred file have changed.
     */
    void fileNameChanged(const QString &fileName);

private:
    explicit ObexTransfer(const QString &path, QObject *parent = 0);

    class ObexTransferPrivate *const d;

    friend class ObexTransferPrivate;
    friend class ObexAgentAdaptor;
    friend class PendingCallPrivate;
};

} // namespace QBluez

#endif // QBLUEZ_OBEXTRANSFER_H
