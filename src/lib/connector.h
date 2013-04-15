#ifndef __CONNECTOR_PROTECTED_H__
#define __CONNECTOR_PROTECTED_H__

#include <mpilote/connector.h>
int connector_createsrvsocket(Connector *this);
ClientAdapter *connector_searchserver(Connector *this, ...);
int connector_waitclient(Connector *this);
ClientAdapter *connector_connected(Connector *this);

#endif
