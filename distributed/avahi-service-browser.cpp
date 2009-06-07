#include "avahi-service-browser.h"
#include "avahi-service-browser.moc"

/*
 * Implementation of interface class AvahiServiceBrowser
 */

AvahiServiceBrowser::AvahiServiceBrowser(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

AvahiServiceBrowser::~AvahiServiceBrowser()
{
}

