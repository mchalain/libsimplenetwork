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

#include <mpilote/debug.h>
#include <mpilote/service.h>
#include <mpilote/clientadapter.h>

typedef struct _ServiceData
{
	enum {Response, Request } m_state;
} ServiceDefault;

static int service_main(Service *this);
static int service_request(Service *this);

void *servicedefault_new(int serviceid)
{
	Service *this = service_new(e_service_tcp);

	this->f_ops.main = service_main;
	this->f_ops.request = service_request;
	ServiceDefault *data = malloc(sizeof(ServiceDefault));
	memset(data, 0, sizeof(ServiceDefault));
	data->m_state= Request;
	this->m_privatedata = data;
	return this;
}

static int service_main(Service *this)
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

static int service_request(Service *this)
{
	char buffer[50];
	int ret;
	int outputfd = 1, inputfd = 0;
	ServiceDefault *data = this->m_privatedata;

	memset(buffer, 0, sizeof(buffer));

	if (this->m_client == NULL)
	{
		log("service not attached to the client\n");
		return -1;
	}
	switch (data->m_state)
	{
		case Request:
			while ((ret = read(inputfd, buffer, sizeof(buffer))) > 0)
			{
				clientadapter_write(this->m_client, buffer, ret);
				if (buffer[0] == '\n')
					break;
			}
			data->m_state = Response;
		break;
		case Response:
			if ((ret = clientadapter_read(this->m_client, buffer, sizeof(buffer))) > 0)
			{
				write(outputfd, buffer, ret);
				if (buffer[0] == '\n')
					break;
			}
			data->m_state = Request;
		break;
	}
	if (ret == -EAGAIN)
	{
		ret = 0;
		printf("try again\n");
	}
	return ret;
}
