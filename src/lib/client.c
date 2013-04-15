/**
 * Project: libclient
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: client.c
 * description: this object manages the main socket and the connections to a server.
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>

#include <mpilote/service.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/connector.h>
#include <mpilote/client.h>
#include <mpilote/debug.h>
#include "connector.h"
#include "service_private.h"

#define MAX(x,y)  (x)>(y)?(x):(y)
#define MIN(x,y) (x)<(y)?(x):(y)

#define MAX_CONNECTOR 10

struct _Client
{
	int m_run;
	Connector *m_connector[MAX_CONNECTOR + 1 ];
	ClientAdapter *m_adapter[MAX_CONNECTOR + 1 ];
	int m_nbconnectors;
};


int client_getmaxfd(Client *this, fd_set *fds);

Client *client_new()
{
	log("%s\n", __FUNCTION__);
	Client *this =malloc(sizeof(Client));
	memset(this, 0, sizeof(Client));
	return this;
}

void client_destroy(Client *this)
{
	int i;
	log("%s\n", __FUNCTION__);
	for (i = 0; i < this->m_nbconnectors; i++)
	{
		connector_destroy(this->m_connector[i]);
		clientadapter_destroy(this->m_adapter[i]);
	}
	free(this);
}

int client_main( Client *this)
{
	int ret;
	fd_set rfds;

	log("%s\n", __FUNCTION__);
	this->m_run = 1;
	do
	{
		int i;

		for (i = 0; i < this->m_nbconnectors; i++)
		{
			ClientAdapter *adapter = this->m_adapter[i];
			ret = service_request(clientadapter_service(adapter));
			if (ret < 0)
			{
				client_closeconnector(this, this->m_connector[i]);
			}
		}

		int maxfd = client_getmaxfd(this, &rfds);
		if (maxfd == 0)
		{
			this->m_run = 0;
			break;
		}
		else
			ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
	}
	while(this->m_run);

	return 0;
}

int client_addconnector(Client *this, Connector *connector)
{
	int ret = 0;
	ClientAdapter *adapter;

	log("%s\n", __FUNCTION__);
	if (this->m_nbconnectors >= MAX_CONNECTOR)
	{
		error("error : %s\n", "too many connectors");
		ret = -1;
	}
	else if ((ret = connector_createsrvsocket(connector)) < 0)
	{
		error("error create socket : %s\n", strerror(errno));
	}
	else	if ((adapter = connector_searchserver(connector)) == NULL)
	{
		error("error search server : %s\n", "server not found");
		ret = -1;
	}
	else
	{
		this->m_connector[this->m_nbconnectors] = connector;
		this->m_adapter[this->m_nbconnectors] = adapter;
		this->m_nbconnectors++;
	}
	return ret;
}

void client_closeconnector(Client *this, Connector *connector)
{
	int j = 0;

	log("%s\n", __FUNCTION__);
	// search the client to close in the list
	while (this->m_connector[j] != connector && this->m_connector[j] != NULL) j++;
	// close the client
	clientadapter_destroy(this->m_adapter[j]);
	connector_destroy(this->m_connector[j]);
	j++;
	// sort the clients' list
	while (this->m_connector[j] != NULL)
	{
		this->m_connector[j - 1] = this->m_connector[j];
		j++;
	}
	this->m_nbconnectors--;
	this->m_connector[this->m_nbconnectors] = NULL;
}

int client_getmaxfd(Client *this, fd_set *fds)
{
	int maxfd = 0;
	int j = 0;
	int fd;

	FD_ZERO(fds);
	while (this->m_connector[j] != NULL)
	{
		Connector *connector = this->m_connector[j];
		fd = connector_getsocket(connector);
		FD_SET(fd, fds);
		maxfd = MAX(maxfd, fd);
		j++;
	}
	return maxfd;
}

void client_stop(Client *this)
{
	int i;
	this->m_run = 0;
	for (i = 0; i < this->m_nbconnectors; i++)
	{
		Connector *connector = this->m_connector[i];
		int fd = connector_getsocket(connector);
		shutdown(fd, SHUT_RDWR);
	}
}

/**
 * API
 **/
