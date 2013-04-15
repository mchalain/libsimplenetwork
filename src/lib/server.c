/**
 * Project: libserver
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: server.c
 * description: this object manages the main socket and the connections from the clients.
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
#include <mpilote/clientadapter.h>
#include <mpilote/server.h>
#include <mpilote/connector.h>
#include <mpilote/debug.h>
#include "connector.h"
#include "service_private.h"

#define MAX(x,y)  (x)>(y)?(x):(y)
#define MIN(x,y) (x)<(y)?(x):(y)

#define MAX_CONNECTOR 10

//log("%s: Leroy was here connectors\n", __FUNCTION__);
/**
 * structures
 **/

struct _Server
{
	int m_run;
	int m_nbconnectors;
	Connector *m_connector[MAX_CONNECTOR + 1 ];
	int m_nbclients;
	ClientAdapter *m_client[MAX_CLIENT + 1];
};


/**
 * internal API
 **/
int server_getmaxfd(Server *this, fd_set *fds);

/**
 * functions definition
 **/
Server *server_new()
{
	Server *this =malloc(sizeof(Server));
	memset(this, 0, sizeof(Server));
	return this;
}

void server_destroy(Server *this)
{
	int i;
	for (i = 0; i < this->m_nbclients; i++)
		server_closeclient(this, this->m_client[i]);
	free(this);
}

int server_main( Server *this)
{
	int ret;
	fd_set rfds;

	log("wait connection\n");
	this->m_run = 1;
	do
	{
		int maxfd = server_getmaxfd(this, &rfds);
		log("maxfd %d\n", maxfd);
		if (maxfd == 0)
		{
			server_stop(this);
			break;
		}
		ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
		if (ret > 0 )
		{
			int i;

			for (i = 0; i < this->m_nbconnectors; i++)
			{
				Connector *connector = this->m_connector[i];
				if (FD_ISSET(connector_getsocket(connector), &rfds) > 0)
				{
					ClientAdapter *client = connector_connected(connector);
					if (client)
					{
						server_addclient(this, client);
					}
					else
					{
						server_closeconnector(this, connector);
						// ugly patch to end if trouble on the UDP socket
						if (i == 0)
							server_stop(this);
					}
					break;
				}
			}

			for (i = 0; i < this->m_nbclients; i++)
			{
				ClientAdapter *client = this->m_client[i];
				if (FD_ISSET(clientadapter_getsocket(client), &rfds))
				{
					ret = service_main(clientadapter_service(client));
					if (ret < 0)
					{
						server_closeclient(this, client);
					}
					break;
				}
			}
		}
		else if (ret < 0)
		{
			error("error on socket :(%d) %s\n", errno, strerror(errno));
			if (errno != EINTR)
				server_stop(this);
		}
	}
	while(this->m_run);

	return 0;
}

void server_stop(Server *this)
{
	int i;
	this->m_run = 0;
	for (i = 0; i < this->m_nbclients; i++)
	{
		ClientAdapter *client = this->m_client[i];
		int fd = clientadapter_getsocket(client);
		shutdown(fd, SHUT_RDWR);
	}
	for (i = 0; i < this->m_nbconnectors; i++)
	{
		Connector *connector = this->m_connector[i];
		int fd = connector_getsocket(connector);
		shutdown(fd, SHUT_RDWR);
	}
}



int server_addconnector(Server *this, Connector *connector)
{
	int ret = -1, i;
	int serviceid = connector_getserviceid(connector);

	for (i = 0; i < MAX_CONNECTOR; i++)
	{
		Connector *cur = this->m_connector[i];
		if (cur == NULL)
		{
			this->m_connector[i] = connector;
			this->m_nbconnectors++;
			if ((ret = connector_createsrvsocket(connector)) < 0)
			{
				error("error : %s\n", strerror(errno));
				return ret;
			}
			ret = connector_waitclient(connector);
			break;
		}
		else if (serviceid == connector_getserviceid(cur))
		{
			//service is already running
			log("service is already running\n");
			break;
		}
	}
	return ret;
}

void server_closeconnector(Server *this, Connector *connector)
{
	int j = 0;

	// search the client to close in the list
	while (this->m_connector[j] != connector && this->m_connector[j] != NULL) j++;
	if (this->m_connector[j] == NULL)
	{
		fprintf(stderr, "connector not found on port %d\n", connector_getport(connector));
		return;
	}
	fprintf(stderr, " close connector on port %d\n", connector_getport(connector));
	// close the client
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
	if (this->m_nbconnectors == 0)
		server_stop(this);
}

int server_addclient(Server *this, ClientAdapter *client)
{
	int ret = -1;

	if (this->m_nbclients < MAX_CLIENT)
	{
		this->m_client[this->m_nbclients] = client;
		this->m_nbclients++;
		ret = 0;
	}
	return ret;
}

void server_closeclient(Server *this, ClientAdapter *client)
{
	int j = 0;

	// search the client to close in the list
	while (this->m_client[j] != client && this->m_client[j] != NULL) j++;
	// close the client
	clientadapter_destroy(this->m_client[j]);
	j++;
	// sort the clients' list
	while (this->m_client[j] != NULL)
	{
		this->m_client[j - 1] = this->m_client[j];
		j++;
	}
	this->m_nbclients--;
	this->m_client[this->m_nbclients] = NULL;
}

int server_getmaxfd(Server *this, fd_set *fds)
{
	int maxfd = 0;
	int j = 0;
	int fd;

	FD_ZERO(fds);
	while (this->m_connector[j] != NULL)
	{
		Connector *connector = this->m_connector[j];
		fd = connector_getsocket(connector);
		if (!FD_ISSET(fd, fds))
		{
			FD_SET(fd, fds);
			maxfd = MAX(maxfd, fd);
		}
		j++;
	}
	j = 0;
	while (this->m_client[j] != NULL)
	{
		ClientAdapter *client = this->m_client[j];
		fd = clientadapter_getsocket(client);
		if (!FD_ISSET(fd, fds))
		{
			FD_SET(fd,fds);
			maxfd = MAX(maxfd, fd);
		}
		j++;
	}
	return maxfd;
}

/**
 * API
 **/
