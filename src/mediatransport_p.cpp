/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * Copyright (C) 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mediatransport_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

static MediaTransport::State stringToState(const QString &state)
{
    if (state == QLatin1String("pending")) {
        return MediaTransport::State::Pending;
    } else if (state == QLatin1String("active")) {
        return MediaTransport::State::Active;
    }
    return MediaTransport::State::Idle;
}

MediaTransportPrivate::MediaTransportPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_path(path)
{
    m_dbusInterface = new QDBusInterface(Strings::orgBluez(),
                                         path,
                                         QStringLiteral("org.bluez.MediaTransport1"),
                                         DBusConnection::orgBluez(),
                                         this);

    DBusConnection::orgBluez().connect(Strings::orgBluez(),
                                       path,
                                       Strings::orgFreedesktopDBusProperties(),
                                       QStringLiteral("PropertiesChanged"),
                                       this,
                                       SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
    init(properties);
}

void MediaTransportPrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(),
                                          m_path,
                                          DBusConnection::orgBluez(),
                                          this);

    m_volume = properties.value(QStringLiteral("Volume")).toUInt();
    m_state = stringToState(properties.value(QStringLiteral("State")).toString());

    // Init properties
    /*m_name = properties.value(QStringLiteral("Name")).toString();
    m_equalizer = stringToEqualizer(properties.value(QStringLiteral("Equalizer")).toString());
    m_repeat = stringToRepeat(properties.value(QStringLiteral("Repeat")).toString());
    m_shuffle = stringToShuffle(properties.value(QStringLiteral("Shuffle")).toString());
    m_status = stringToStatus(properties.value(QStringLiteral("Status")).toString());
    m_track = variantToTrack(properties.value(QStringLiteral("Track")));
    m_position = properties.value(QStringLiteral("Position")).toUInt();
    */
}

void MediaTransportPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface != Strings::orgBluezMediaTransport1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Volume")) {
            m_volume = value.toUInt();
            Q_EMIT q.data()->volumeChanged(m_volume);
        } else if (property == QLatin1String("State")) {
            m_state = stringToState(value.toString());
            Q_EMIT q.data()->stateChanged(m_state);
        }

/*
        if (property == QLatin1String("Name")) {
            PROPERTY_CHANGED(m_name, toString, nameChanged);
        } else if (property == QLatin1String("Equalizer")) {
            PROPERTY_CHANGED2(m_equalizer, stringToEqualizer(value.toString()), equalizerChanged);
        } else if (property == QLatin1String("Repeat")) {
            PROPERTY_CHANGED2(m_repeat, stringToRepeat(value.toString()), repeatChanged);
        } else if (property == QLatin1String("Shuffle")) {
            PROPERTY_CHANGED2(m_shuffle, stringToShuffle(value.toString()), shuffleChanged);
        } else if (property == QLatin1String("Status")) {
            PROPERTY_CHANGED2(m_status, stringToStatus(value.toString()), statusChanged);
        } else if (property == QLatin1String("Position")) {
            PROPERTY_CHANGED(m_position, toUInt, positionChanged);
        } else if (property == QLatin1String("Track")) {
            m_track = variantToTrack(value);
            Q_EMIT q.data()->trackChanged(m_track);
        }
        */
    }

    for (const QString &property : invalidated) {
        /*
        if (property == QLatin1String("Name")) {
            PROPERTY_INVALIDATED(m_name, QString(), nameChanged);
        } else if (property == QLatin1String("Equalizer")) {
            PROPERTY_INVALIDATED(m_equalizer, MediaTransport::EqualizerOff, equalizerChanged);
        } else if (property == QLatin1String("Repeat")) {
            PROPERTY_INVALIDATED(m_repeat, MediaTransport::RepeatOff, repeatChanged);
        } else if (property == QLatin1String("Shuffle")) {
            PROPERTY_INVALIDATED(m_shuffle, MediaTransport::ShuffleOff, shuffleChanged);
        } else if (property == QLatin1String("Status")) {
            PROPERTY_INVALIDATED(m_status, MediaTransport::Error, statusChanged);
        } else if (property == QLatin1String("Position")) {
            PROPERTY_INVALIDATED(m_position, 0, positionChanged);
        } else if (property == QLatin1String("Track")) {
            m_track = variantToTrack(QVariant());
            Q_EMIT q.data()->trackChanged(m_track);
        }
        */
    }
}

} // namespace BluezQt
