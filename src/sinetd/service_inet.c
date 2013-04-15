/**
 * Project: libserviceinet
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

#include <service.h>
#include <clientadapter.h>

static int service_main(Service *this);

void *serviceinet_new(int serviceid)
{
	Service *this = malloc(sizeof(Service));
	if (serviceid)
		this->m_port = serviceid;
	else
		this->m_port = 80;
	this->m_type = SOCK_STREAM;
	this->m_protocol = IPPROTO_TCP;
	this->m_client = NULL;
	this->m_opts.f_setfl = O_NONBLOCK;
	this->m_opts.so_reuseaddr |= 0x01;
	this->m_opts.so_keepalive |= 0x01;
	this->f_ops.main = service_main;
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
