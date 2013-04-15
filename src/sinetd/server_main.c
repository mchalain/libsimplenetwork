/**
 * Project: testserver
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: main.c
 * description: main fonction to manage a simple server.
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/server.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/connector.h>
#include "service_inet.h"

/**
 * functions declaration
 **/

Server *g_server;

/**
 * signal handlers
 **/
void stop(void)
{
	server_stop(g_server);
}
void terminate(int sig)
{
    if (sig == SIGTERM || sig == SIGINT)
        stop();
}

/**
 * main function
 **/
int main(int argc, char ** argv)
{
	int ret;
	int serviceid = 80;

	g_server = server_new();
	atexit(stop);
	signal(SIGINT, terminate);

	ServicesFactory *factory = servicesfactory_new();
	servicesfactory_add(factory, serviceid, NULL, serviceinet_new);

	Connector *connector = connector_new(factory, serviceid);
	connector_readparameters(connector, argv, argc);

	server_addconnector(g_server, connector);

	ret = server_main(g_server);

	server_destroy(g_server);
	servicesfactory_destroy(factory);
	return ret;
}
