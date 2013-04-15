/**
 * Project: libserver
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: servicesfactory.c
 * description: this object manages the service behind the client socket.
 *
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <dlfcn.h>

#include <mpilote/servicesfactory.h>
#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>
#include "service_private.h"

#define MAX_SERVICES 4
#define SERVICEID_MIN 9000
typedef struct
{
	int m_serviceid;
	Service_New *f_service_new;
} ServiceEntry;

struct _ServicesFactory
{
	ServiceEntry *m_service[MAX_SERVICES];
};

static Service_New *servicesfactory_loadlib(ServicesFactory *this, char *libpath);

ServicesFactory *servicesfactory_new()
{
	ServicesFactory *this = (ServicesFactory *)malloc(sizeof(ServicesFactory));
	memset(this, 0, sizeof(ServicesFactory));
	return this;
}

int servicesfactory_add(ServicesFactory *this, char *libpath, Service_New *service_new)
{
	int i, serviceid = SERVICEID_MIN;
	ServiceEntry *entry = NULL;

	
	for (i = 0; i < MAX_SERVICES; i++)
	{
		// generate a new serviceid
		if (this->m_service[i] && this->m_service[i]->m_serviceid >= serviceid)
		{
			serviceid = this->m_service[i]->m_serviceid + 1 ;
		}
		// look for a empty entry
		else if (this->m_service[i] == NULL) break;
	}
	
	if (i == MAX_SERVICES)
	{
		return -1;
	}
	else
	{
		entry = malloc(sizeof(ServiceEntry));
		memset(entry, 0, sizeof(ServiceEntry));
		if (libpath != NULL)
			entry->f_service_new = servicesfactory_loadlib(this, libpath);
		else
			entry->f_service_new = service_new;

		if (entry->f_service_new != NULL)
		{
			entry->m_serviceid = serviceid;
			this->m_service[i] = entry;
		}
		else
		{
			free(entry);
			return -1;
		}
	}
	return serviceid;
}

int servicesfactory_getserviceid(ServicesFactory *this, char *name)
{
	int i;
	int serviceid = 0;
	for (i = 0; i < MAX_SERVICES && this->m_service[i] ; i++)
	{
		if (this->m_service[i] != NULL && this->m_service[i]->f_service_new != NULL)
		{
			Service *service = NULL;
			service = this->m_service[i]->f_service_new(this->m_service[i]->m_serviceid);

			if (service && service->f_ops.getname)
			{
				log("service %s in place %d\n", service->f_ops.getname(service), i);
				char *sname = service->f_ops.getname(service);
				if (sname && !strncmp(name, sname, strlen(sname)))
				{
					
					serviceid = this->m_service[i]->m_serviceid;
				}
			}
			service_destroy(service);
		}
	}
	return serviceid;
}

Service *servicesfactory_createservice(ServicesFactory *this, int serviceid)
{
	Service *service = NULL;
	int i;

	log("looks for service %d ", serviceid);
	for (i = 0; i < MAX_SERVICES; i++)
		if (this->m_service[i] && this->m_service[i]->m_serviceid == serviceid) break;
	if (this->m_service[i] != NULL && this->m_service[i]->f_service_new != NULL)
	{
		log("found\n");
		service = this->m_service[i]->f_service_new(this->m_service[i]->m_serviceid);
	}
	else
	{
		log("not found\n");
	}
	return service;
}

void servicesfactory_destroy(ServicesFactory *this)
{
	int i;
	for (i = 0; i < MAX_SERVICES; i++)
		if (this->m_service[i])
			free(this->m_service[i]);
	free(this);
}

static Service_New *servicesfactory_loadlib(ServicesFactory *this, char *libpath)
{
	void *handle;
	Service_New **func;
	void (*init)();
	handle = dlopen(libpath, RTLD_LAZY);
	if (!handle)
	{
		error("error on plugin loading err : %s\n",dlerror());
		return (void *)-1;
	}
	init = dlsym(handle, "servicelib_initiate");
	if (init)
		(*init)();
	func = dlsym(handle, "servicelib_new");
	if (!func)
		error("error no servicelib_new function: %s\n", dlerror());
	return *func;
}
