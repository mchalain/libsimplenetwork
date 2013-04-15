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
#include <errno.h>
#include <signal.h>

#include <mpilote/service.h>
#include <mpilote/connector.h>
#include <mpilote/client.h>
#include "service_default.h"

#define MAX_CLIENT 255
#define CREATE_ADDR(a,b,c,d) ((long)a|(long)b>>8|(long)c>>16|(long)d>>24)
/**
 * structures
 **/
/**
 * global variables
 **/
Client *g_client;
/**
 * functions declaration
 **/

void stop(void)
{
	client_stop(g_client);
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

	g_client = client_new();
	atexit(stop);
	signal(SIGINT, terminate);

	ServicesFactory *factory = servicesfactory_new();
	serviceid = servicesfactory_add(factory, NULL, servicedefault_new);

	Connector *connector = connector_new(factory, serviceid);
	connector_readparameters(connector, argv, argc);

	client_addconnector(g_client, connector);

	ret = client_main(g_client);

	client_destroy(g_client);
	servicesfactory_destroy(factory);
	return ret;
}

/**
 * functions definition
 **/