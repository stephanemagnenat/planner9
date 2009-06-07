#include "avahi-server.h"
#include "avahi-server.moc"

/*
 * Implementation of interface class AvahiServer
 */

AvahiServer::AvahiServer(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
	qDBusRegisterMetaType<QList<QByteArray> >();
}

AvahiServer::~AvahiServer()
{
}

