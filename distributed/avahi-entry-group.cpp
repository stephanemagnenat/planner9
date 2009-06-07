#include "avahi-entry-group.h"
#include "avahi-entry-group.moc"

/*
 * Implementation of interface class AvahiEntryGroup
 */

AvahiEntryGroup::AvahiEntryGroup(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

AvahiEntryGroup::~AvahiEntryGroup()
{
}
