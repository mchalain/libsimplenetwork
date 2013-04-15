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
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <mpilote/service.h>
#include <mpilote/server.h>
#include <mpilote/connector.h>
#include <mpilote/debug.h>
#include "service_private.h"

struct _Connector
{
	int m_socket;
	int m_port;
	int m_type;
	int m_protocol;
	char *m_address;
	void *m_services;
	int m_serviceid;
	int m_clientthreaded;
	struct sockaddr_storage m_peeraddr;
};

static ClientAdapter *connector_createadapter(Connector *this, int clientfd);
static int connector_acceptclient(Connector *this, ...);

Connector *connector_new(ServicesFactory *factory, int serviceid)
{
	log("%s\n", __FUNCTION__);
	Connector *this = malloc(sizeof(Connector));
	memset(this, 0, sizeof(Connector));
	this->m_port = serviceid;
	this->m_serviceid = serviceid;
	this->m_services = factory;
	//set the default service
	Service * service = servicesfactory_createservice(this->m_services, serviceid);
	if (service != NULL)
	{
		service_getprotocol(service, &this->m_port, &this->m_type, &this->m_protocol);
		service_destroy(service);
	}
	return this;
}

void connector_destroy(Connector *this)
{
	if (this->m_address)
		free(this->m_address);
	shutdown(this->m_socket,SHUT_RDWR);
	close(this->m_socket);

	free(this);
}

int connector_findport(Connector *this)
{

/*
	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if (s != 0) {
		error( "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// getaddrinfo() returns a list of address structures.
	//   Try each address until we successfully bind(2).
	//   If socket(2) (or bind(2)) fails, we (close the socket
	//   and) try the next address.

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;		   // Success

		close(sfd);
	}

	if (rp == NULL) {		   // No address succeeded
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);	   //No longer needed

*/
	return 80;
}

int connector_getserviceid(Connector *this)
{
	return this->m_serviceid;
}

void connector_setport(Connector *this, int port)
{
	this->m_port = port;
}

int connector_getport(Connector *this)
{
	return this->m_port;
}

void connector_setaddress(Connector *this, char *address)
{
	if (!address)
		return;
	int len = strlen(address);
	if (len <= 0)
		return;
	this->m_address = malloc(len+ 1);
	strcpy(this->m_address, address);
}

int connector_getsocket(Connector *this)
{
	return this->m_socket;
}

ClientAdapter *connector_connected(Connector *this)
{
	ClientAdapter *client;
	int clientfd = 0;

	log("%s\n", __FUNCTION__);
	if (this->m_protocol == IPPROTO_TCP)
	{
		log ("new TCP client\n");
		clientfd = connector_acceptclient(this);
	}
	else
	{
		log  ("UDP request\n");
		clientfd = this->m_socket;
	}
	client = connector_createadapter(this, clientfd);
	if ((client) && (this->m_protocol == IPPROTO_TCP))
		client = clientadapter_start(client);
	return client;
}

int connector_waitclient(Connector *this)
{
	int ret = 0;
	struct sockaddr_in address = {0};
	int broadcast = 0;

	log("%s\n", __FUNCTION__);
	address.sin_family = AF_INET;
	address.sin_port = htons(this->m_port);
	

	if (this->m_address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(!strcmp(this->m_address, BROADCAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else if(!strcmp(this->m_address, MULTICAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else
		inet_aton(this->m_address, &address.sin_addr);

	if (setsockopt(this->m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		error("error broadcast opt: %s\n", strerror(errno));
		close(this->m_socket);
		this->m_socket = 0;
	}
	else if ((ret = bind(this->m_socket, (struct sockaddr *)&address, sizeof(struct sockaddr_in))) < 0)
	{
		error("error bind: %s\n", strerror(errno));
	}
	else if ((this->m_type == SOCK_STREAM) && (ret = listen(this->m_socket, 10) < 0))
	{
		error("error listen: %s\n", strerror(errno));
	}
	else if ((ret = fcntl(this->m_socket, F_SETFL, O_NONBLOCK)) < 0)
	{
		error("error nonblock: %s\n", strerror(errno));
	}
	return ret;
}

static int connector_acceptclient(Connector *this, ...)
{
	log("%s\n", __FUNCTION__);

	int clientfd = accept(this->m_socket, NULL, 0);
	if (clientfd < 0)
	{
		error("error accept: %s\n", strerror(errno));
		return -errno;
	}
	else
	{
		return clientfd;
	}
}

ClientAdapter *connector_searchserver(Connector *this, ...)
{
	struct sockaddr_in address = {0};
	ClientAdapter * client = NULL;
	int broadcast = 0;

	log("%s\n", __FUNCTION__);
	address.sin_family = AF_INET;
	address.sin_port = htons(this->m_port);
	if (this->m_address == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(!strcmp(this->m_address, BROADCAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else if(!strcmp(this->m_address, MULTICAST))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast = 1;
	}
	else
		inet_pton(address.sin_family, this->m_address, (void *)&address.sin_addr.s_addr);

	if (connect(this->m_socket, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		error("error connect: %s\n", strerror(errno));
	}
	else if (setsockopt(this->m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		error("error broadcast opt: %s\n", strerror(errno));
		close(this->m_socket);
		this->m_socket = 0;
	}
	else
	{
		client = connector_createadapter(this, this->m_socket);
		if (client)
			client = clientadapter_start(client);
	}
	return client;
}

static ClientAdapter *connector_createadapter(Connector *this, int clientfd)
{
	ClientAdapter * client = NULL;
	log("%s\n", __FUNCTION__);
	if (clientfd)
	{
		Service *service = servicesfactory_createservice(this->m_services, this->m_serviceid);
		if (service != NULL)
		{
			service_changeport(service, this->m_port);
			client = clientadapter_new(service, clientfd);
		}
	}
	return client;
}

int connector_createsrvsocket(Connector *this)
{
	int ret;
	int fd;
	int yes = 1;
	int protocol;
	int fcnt;

	log("%s\n", __FUNCTION__);
	if (this->m_type == SOCK_STREAM)
		log("new SOCK_STREAM on port %d \n", this->m_port);
	else
		log("new SOCK_DGRAM on port %d \n", this->m_port);

	if (this->m_protocol == IPPROTO_UDP)
	{
		protocol = 0;
     /* use setsockopt() to request that the kernel join a multicast group */
/*
     mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);
     if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
	  error("setsockopt");
	  exit(1);
     }
*/
	
	}
	else
		protocol = this->m_protocol;
	if ((fd = socket(PF_INET, this->m_type, protocol)) < 0)
	{
		error("error socket: %s\n", strerror(errno));
		ret = fd;
	}
	else if ((ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) < 0)
	{
		error("error reuse opt: %s\n", strerror(errno));
		close(fd);
		fd = 0;
	}
	else if ((ret = fcntl(fd,F_GETFL,0)) < 0)
	{
	}
	else if ((ret = fcntl(fd,F_SETFL,ret | O_NONBLOCK)) < 0)
	{
	}
	else
	{
		this->m_socket = fd;
		log("new socket : %d\n", this->m_socket);
	}
	return ret;
}

void connector_readparameters(Connector *this, char ** p_argv, int p_argc)
{
	int i;
	log("%s\n", __FUNCTION__);
	for (i = 1; i < p_argc; i++)
	{
		if ((strncmp(p_argv[i], "--port", 6) == 0) && (i+1 < p_argc))
		{
			this->m_port = atoi(p_argv[++i]);
		}
		else if ((strncmp(p_argv[i], "--address", 10) == 0) && (i+1 < p_argc))
		{
			this->m_address = malloc(strlen(p_argv[++i]) + 1);
			strcpy(this->m_address, p_argv[i]);
		}
		else if ((strncmp(p_argv[i], "--service", 10) == 0) && (i+1 < p_argc))
		{
			int serviceid = servicesfactory_getserviceid(this->m_services, p_argv[++i]);
			Service *service = servicesfactory_createservice(this->m_services, serviceid);
			if ( service != NULL)
			{
				service_getprotocol(service, &this->m_port, &this->m_type, &this->m_protocol);
				service_destroy(service);
			}
		}
		else if ((strncmp(p_argv[i], "--protocol", 10) == 0) && (i+1 < p_argc))
		{
			++i;
			if (strncmp(p_argv[i], "tcp", 3) == 0)
			{
				this->m_type = SOCK_STREAM;
				this->m_protocol = IPPROTO_TCP;
			}
			else if (strncmp(p_argv[i], "udp", 3) == 0)
			{
				this->m_type = SOCK_DGRAM;
				this->m_protocol = IPPROTO_UDP;
			}
			else
			{
				struct protoent *pprotoent = getprotobyname(p_argv[i]);
				this->m_type = SOCK_RAW;
				this->m_protocol = pprotoent->p_proto;
				endprotoent();
			}
		}
		else if ((strncmp(p_argv[i], "--help", 6) == 0))
		{
			fprintf(stderr, "%s: ", p_argv[0]);
			fprintf(stderr, "--port <NUM>");
			fprintf(stderr, "--service <NAME>");
			fprintf(stderr, "--protocol [tcp|udp]");
		}
	}
}
