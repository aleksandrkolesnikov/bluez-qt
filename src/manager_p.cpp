#include "manager_p.h"
#include "device.h"
#include "adapter.h"
#include "adapter_p.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

using namespace QBluez;

ManagerPrivate::ManagerPrivate(Manager *parent)
    : QObject(parent)
    , q(parent)
    , m_dbusObjectManager(0)
    , m_bluezAgentManager(0)
    , m_usableAdapter(0)
    , m_bluezRunning(false)
    , m_initialized(false)
{
    qDBusRegisterMetaType<DBusManagerStruct>();
    qDBusRegisterMetaType<QVariantMapMap>();

    // Keep an eye on bluez service
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(QStringLiteral("org.bluez"), QDBusConnection::systemBus(),
            QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, [ = ]() {
        m_bluezRunning = true;
        initialize();
    });

    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, [ = ]() {
        m_bluezRunning = false;
        clear();
    });

    // Update the current state of bluez service
    if (QDBusConnection::systemBus().isConnected()) {
        QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                            QStringLiteral("/"),
                            QStringLiteral("org.freedesktop.DBus"),
                            QStringLiteral("NameHasOwner"));
        QList<QVariant> args;
        args.append(QStringLiteral("org.bluez"));
        call.setArguments(args);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(call));

        connect(watcher, &QDBusPendingCallWatcher::finished, [ = ]() {
            const QDBusPendingReply<bool> &reply = *watcher;

            if (reply.isError()) {
                qWarning() << "Error:" << reply.error().message();
            } else {
                m_bluezRunning = reply.isValid() && reply.value();
                initialize();
            }
        });
    }
}

void ManagerPrivate::initialize()
{
    if (!m_bluezRunning || m_initialized) {
        return;
    }

    m_dbusObjectManager = new DBusObjectManager(QStringLiteral("org.bluez"), QStringLiteral("/"),
            QDBusConnection::systemBus(), this);

    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesAdded,
            this, &ManagerPrivate::interfacesAdded);
    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesRemoved,
            this, &ManagerPrivate::interfacesRemoved);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_dbusObjectManager->GetManagedObjects(), this);

    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ]() {
        const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;

        if (reply.isError()) {
            qWarning() << "Error:" << reply.error().message();
        } else {
            DBusManagerStruct::const_iterator it;
            const DBusManagerStruct &managedObjects = reply.value();

            for (it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
                const QString &path = it.key().path();
                const QVariantMapMap &interfaces = it.value();

                if (interfaces.contains(QStringLiteral("org.bluez.Adapter1"))) {
                    m_adapters.insert(path, new Adapter(path, this));
                } else if (interfaces.contains(QStringLiteral("org.bluez.Device1"))) {
                    const QString &adapterPath = it.value().value(QStringLiteral("org.bluez.Device1")).value(QStringLiteral("Adapter")).value<QDBusObjectPath>().path();
                    Adapter *adapter = m_adapters.value(adapterPath);
                    Q_ASSERT(adapter);
                    Device *device = new Device(path, adapter, this);
                    adapter->d->addDevice(device);
                    m_devices.insert(path, device);
                } else if (interfaces.contains(QStringLiteral("org.bluez.AgentManager1"))) {
                    m_bluezAgentManager = new BluezAgentManager(QStringLiteral("org.bluez"), path, QDBusConnection::systemBus(), this);
                }
            }

            Q_ASSERT(m_bluezAgentManager);

            m_initialized = true;
            Q_EMIT q->operationalChanged(true);
        }

        delete watcher;
    });
}

void ManagerPrivate::clear()
{
    m_initialized = false;

    qDeleteAll(m_adapters);
    m_adapters.clear();

    delete m_dbusObjectManager;
    m_dbusObjectManager = 0;

    delete m_bluezAgentManager;
    m_bluezAgentManager = 0;

    Q_EMIT q->operationalChanged(false);
}

void ManagerPrivate::interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces)
{
    const QString &path = objectPath.path();
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == QLatin1String("org.bluez.Adapter1")) {
            Adapter *adapter = new Adapter(path, this);
            m_adapters.insert(path, adapter);
            Q_EMIT q->adapterAdded(adapter);
        } else if (it.key() == QLatin1String("org.bluez.Device1")) {
            const QString &adapterPath = it.value().value(QStringLiteral("Adapter")).value<QDBusObjectPath>().path();
            Adapter *adapter = m_adapters.value(adapterPath);
            Q_ASSERT(adapter);
            Device *device = new Device(path, adapter, this);
            adapter->d->addDevice(device);
            m_devices.insert(path, device);
        }
    }
}

void ManagerPrivate::interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    const QString &path = objectPath.path();

    Q_FOREACH (const QString &interface, interfaces) {
        if (interface == QLatin1String("org.bluez.Adapter1")) {
            Adapter *adapter = m_adapters.take(path);
            Q_EMIT q->adapterRemoved(adapter);
            delete adapter;
        } else if (interface == QLatin1String("org.bluez.Device1")) {
            Device *device = m_devices.value(path);
            if (device) {
                device->adapter()->d->removeDevice(device);
                m_devices.remove(path);
                delete device;
                break;
            }
        }
    }
}
