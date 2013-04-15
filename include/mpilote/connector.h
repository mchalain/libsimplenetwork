#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

struct _Connector;
typedef struct _Connector Connector;

#include <mpilote/servicesfactory.h>

#define BROADCAST "BROADCAST"
#define MULTICAST "MULTICAST"

Connector *connector_new(ServicesFactory *factory, int serviceid);
void connector_destroy(Connector *this);
int connector_getserviceid(Connector *this);
void connector_setport(Connector *this, int port);
int connector_getport(Connector *this);
void connector_setaddress(Connector *this, char *address);
void connector_readparameters(Connector *this, char ** p_argv, int p_argc);
int connector_getsocket(Connector *this);

#endif