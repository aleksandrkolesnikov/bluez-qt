#include "device_p.h"
#include "device.h"
#include "adapter.h"
#include "utils_p.h"

namespace QBluez
{

DevicePrivate::DevicePrivate(const QString &path, const QVariantMap &properties, Adapter *adapter, Device *parent)
    : QObject(parent)
    , q(parent)
    , m_dbusProperties(0)
    , m_deviceClass(0)
    , m_appearance(0)
    , m_paired(false)
    , m_trusted(false)
    , m_blocked(false)
    , m_legacyPairing(false)
    , m_rssi(-1)
    , m_connected(false)
    , m_adapter(adapter)
{
    m_bluezDevice = new BluezDevice(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void DevicePrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezDevice->path(),
                                          DBusConnection::orgBluez(), this);

    // QueuedConnection is important here - see AdapterPrivate::initProperties
    connect(m_dbusProperties, &DBusProperties::PropertiesChanged,
            this, &DevicePrivate::propertiesChanged, Qt::QueuedConnection);

    // Init properties
    m_address = properties.value(QStringLiteral("Address")).toString();
    m_name = properties.value(QStringLiteral("Name")).toString();
    m_alias = properties.value(QStringLiteral("Alias")).toString();
    m_deviceClass = properties.value(QStringLiteral("Class")).toUInt();
    m_appearance = properties.value(QStringLiteral("Appearance")).toUInt();
    m_icon = properties.value(QStringLiteral("Icon")).toString();
    m_paired = properties.value(QStringLiteral("Paired")).toBool();
    m_trusted = properties.value(QStringLiteral("Trusted")).toBool();
    m_blocked = properties.value(QStringLiteral("Blocked")).toBool();
    m_legacyPairing = properties.value(QStringLiteral("LegacyPairing")).toBool();
    m_rssi = properties.value(QStringLiteral("RSSI")).toInt();
    m_connected = properties.value(QStringLiteral("Connected")).toBool();
    m_uuids = stringListToUpper(properties.value(QStringLiteral("UUIDs")).toStringList());
    m_modalias = properties.value(QStringLiteral("Modalias")).toString();
}

QDBusPendingReply<> DevicePrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezDevice1(), name, QDBusVariant(value));
}

// Make sure not to emit propertyChanged signal when the property already contains changed value
#define PROPERTY_CHANGED(var, type_cast, signal) \
    if (var != value.type_cast()) { \
        var = value.type_cast(); \
        Q_EMIT q->signal(var); \
    }

void DevicePrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(interface)
    Q_UNUSED(invalidated)

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Name")) {
            namePropertyChanged(value.toString());
        } else if (property == QLatin1String("Alias")) {
            aliasPropertyChanged(value.toString());
        } else if (property == QLatin1String("Class")) {
            classPropertyChanged(value.toUInt());
        } else if (property == QLatin1String("Appearance")) {
            PROPERTY_CHANGED(m_appearance, toUInt, appearanceChanged);
        } else if (property == QLatin1String("Icon")) {
            PROPERTY_CHANGED(m_icon, toString, iconChanged);
        } else if (property == QLatin1String("Paired")) {
            PROPERTY_CHANGED(m_paired, toBool, pairedChanged);
        } else if (property == QLatin1String("Trusted")) {
            PROPERTY_CHANGED(m_trusted, toBool, trustedChanged);
        } else if (property == QLatin1String("Blocked")) {
            PROPERTY_CHANGED(m_blocked, toBool, blockedChanged);
        } else if (property == QLatin1String("LegacyPairing")) {
            PROPERTY_CHANGED(m_legacyPairing, toBool, legacyPairingChanged);
        } else if (property == QLatin1String("RSSI")) {
            PROPERTY_CHANGED(m_rssi, toInt, rssiChanged);
        } else if (property == QLatin1String("Connected")) {
            PROPERTY_CHANGED(m_connected, toBool, connectedChanged);
        } else if (property == QLatin1String("Modalias")) {
            PROPERTY_CHANGED(m_modalias, toString, modaliasChanged);
        } else if (property == QLatin1String("UUIDs")) {
            uuidsPropertyChanged(stringListToUpper(value.toStringList()));
        }
    }

    Q_EMIT q->deviceChanged(q);
    Q_EMIT m_adapter->deviceChanged(q);
}

void DevicePrivate::namePropertyChanged(const QString &value)
{
    if (m_name != value) {
        m_name = value;
        Q_EMIT q->nameChanged(m_name);
        Q_EMIT q->friendlyNameChanged(q->friendlyName());
    }
}

void DevicePrivate::aliasPropertyChanged(const QString &value)
{
    if (m_alias != value) {
        m_alias = value;
        Q_EMIT q->aliasChanged(m_alias);
        Q_EMIT q->friendlyNameChanged(q->friendlyName());
    }
}

void DevicePrivate::classPropertyChanged(quint32 value)
{
    if (m_deviceClass != value) {
        m_deviceClass = value;
        Q_EMIT q->deviceClassChanged(m_deviceClass);
        Q_EMIT q->deviceTypeChanged(q->deviceType());
    }
}

void DevicePrivate::uuidsPropertyChanged(const QStringList &value)
{
    if (m_uuids != value) {
        m_uuids = value;
        Q_EMIT q->uuidsChanged(m_uuids);
    }
}

#undef PROPERTY_CHANGED

} // namespace QBluez
