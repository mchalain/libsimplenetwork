#ifndef __SERVICESFACTORY_H__
#define __SERVICESFACTORY_H__

struct _ServicesFactory;
typedef struct _ServicesFactory ServicesFactory;
#include <mpilote/service.h>

typedef Service *(Service_New)(int serviceid);

ServicesFactory *servicesfactory_new();
void servicesfactory_destroy(ServicesFactory *this);
/**
 * @return the serviceid of the new service or -1 for error
 **/
int servicesfactory_add(ServicesFactory *this, char *libpath, Service_New *service_new);
int servicesfactory_getserviceid(ServicesFactory *this, char *name);
Service *servicesfactory_createservice(ServicesFactory *this, int serviceid);
#endif