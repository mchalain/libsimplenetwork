#ifndef __SERVICE_H__FOR_SERVER
#define __SERVICE_H__FOR_SERVER

#include <mpilote/clientadapter.h>
#include <mpilote/service.h>

typedef enum
{
	SERVICEOPT_F_SETFL,
	SERVICEOPT_SO_REUSEADDR,
	SERVICEOPT_SO_KEEPALIVE,
	SERVICEOPT_SO_BROADCAST,
} ServiceOpt;

int service_attach(Service *this, ClientAdapter *client);
int service_getprotocol(Service *this, int *port, int *type, int *protocol);
int service_getsocketopt(Service *this, ServiceOpt option, int *value);
int service_main(Service *this);
int service_request(Service *this);
void service_destroy(Service *this);
int service_isstream(Service *this);
int service_changeport(Service *this, int newport);
#endif
