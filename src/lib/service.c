/**
 * Project: libservicedefault
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: service_default.c
 * description: this object gives simple service to the server.
 *     to create a new server, copy this file and modify the following functions:
 *       - service_new defines the port and protocol
 *       - service_main defines the requests treatments
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/debug.h>
#include "service_private.h"

static int _service_main(Service *this);
static int _service_request(Service *this);

/**
 * this function is created for each service
 **/
Service *service_new(E_ServiceType servicetype)
{
	Service *this = malloc(sizeof(Service));
	memset(this, 0, sizeof(Service));
	switch (servicetype)
	{
		case e_service_tcp:
		{
			this->m_type = SOCK_STREAM;
			this->m_protocol = IPPROTO_TCP;
			this->m_client = NULL;
			this->m_opts.f_setfl = O_NONBLOCK;
			this->m_opts.so_reuseaddr |= 0x01;
			this->m_opts.so_keepalive |= 0x01;
			this->m_opts.so_broadcast |= 0x00;
		}
		break;
		case e_service_udp:
		{
			this->m_type = SOCK_DGRAM;
			this->m_protocol = IPPROTO_UDP;
			this->m_client = NULL;
			this->m_opts.f_setfl = O_NONBLOCK;
			this->m_opts.so_reuseaddr |= 0x01;
			this->m_opts.so_keepalive |= 0x01;
			this->m_opts.so_broadcast |= 0x00;
		}
		break;
		case e_service_broadcast:
		{
			this->m_type = SOCK_DGRAM;
			this->m_protocol = IPPROTO_UDP;
			this->m_client = NULL;
			this->m_opts.f_setfl = O_NONBLOCK;
			this->m_opts.so_reuseaddr |= 0x01;
			this->m_opts.so_keepalive |= 0x01;
			this->m_opts.so_broadcast |= 0x01;
		}
		break;
		case e_service_multicast:
		{
			this->m_type = SOCK_DGRAM;
			this->m_protocol = IPPROTO_UDP;
			this->m_client = NULL;
			this->m_opts.f_setfl = O_NONBLOCK;
			this->m_opts.so_reuseaddr |= 0x01;
			this->m_opts.so_keepalive |= 0x01;
			this->m_opts.so_broadcast |= 0x01;
		}
		break;
	}
	return this;
}

int service_main(Service *this)
{
	if (this->f_ops.main)
		return this->f_ops.main(this);
	else
		return _service_main(this);
}

static int _service_main(Service *this)
{
	char buffer[50];
	int ret;
	int outputfd = 1, inputfd = 0;

	memset(buffer, 0, sizeof(buffer));

	if (this->m_client == NULL)
	{
		log("service not attached to the client\n");
		return -1;
	}
	if ((ret = clientadapter_read(this->m_client, buffer, sizeof(buffer))) > 0)
	{
		write(outputfd, buffer, ret);
		if (buffer[ret -1] == '\n')
		{
			while ((ret = read(inputfd, buffer, sizeof(buffer))) > 0)
			{
				clientadapter_write(this->m_client, buffer, ret);
				if (buffer[0] == '\n')
					break;
			}
		}

	}
	if (ret == -EAGAIN)
	{
		ret = 0;
		printf("try again\n");
	}
	return ret;
}

int service_request(Service *this)
{
	if (this->f_ops.request)
		return this->f_ops.request(this);
	else
		return _service_request(this);
}

static int _service_request(Service *this)
{
	return 0;
}

int service_attach(Service *this, ClientAdapter *client)
{
	this->m_client = client;
	return 0;
}

int service_isstream(Service *this)
{
	return (this->m_type ==SOCK_STREAM)? 1:0;
}

void service_destroy(Service *this)
{
	if (this->f_ops.destroy)
		this->f_ops.destroy(this);
	else
		free(this);
}

int service_getprotocol(Service *this, int *port, int *type, int *protocol)
{
	*port = this->m_port;
	*type = this->m_type;
	*protocol = this->m_protocol;
	return 0;
}

int service_getsocketopt(Service *this, ServiceOpt option, int *value)
{
	switch(option)
	{
		case SERVICEOPT_F_SETFL:
			// to continu to read the socket after receive
			*value = this->m_opts.f_setfl;
		break;
		case SERVICEOPT_SO_REUSEADDR:
			*value = this->m_opts.so_reuseaddr;
		break;
		case SERVICEOPT_SO_KEEPALIVE:
			*value = this->m_opts.so_keepalive;
		break;
		case SERVICEOPT_SO_BROADCAST:
			*value = this->m_opts.so_broadcast;
		break;
		default:
			*value = 0;
	}
	return 0;
}

int service_changeport(Service *this, int newport)
{
	//this function is available only if client is not attached
	if (this->m_client == NULL)
		this->m_port = newport;
	return this->m_port;
}