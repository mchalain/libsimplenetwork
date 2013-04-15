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
#include <string.h>

#include <mpilote/service.h>
#include <mpilote/connector.h>
#include <mpilote/client.h>
#include <mpilote/debug.h>

#define DEFAULTNAME "test"
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
extern void *servicepairing_new(int serviceid);
struct _discover
{
	char *m_name;
	int m_port;
} g_discover;

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
	int serviceid;
	int i;

	g_client = client_new();
	atexit(stop);
	signal(SIGINT, terminate);

	ServicesFactory *factory = servicesfactory_new();
	serviceid = servicesfactory_add(factory, NULL, servicepairing_new);

	Connector *connector = connector_new(factory, serviceid);
	connector_readparameters(connector, argv, argc);

	memset(&g_discover, 0, sizeof(struct _discover));
	g_discover.m_port = 9001;
	for (i = 1; i < argc; i++)
	{
		if ((strncmp(argv[i], "--discoverport", 14) == 0) && (i+1 < argc))
		{
			g_discover.m_port = atoi(argv[++i]);
		}
		else if ((strncmp(argv[i], "--discovername", 14) == 0) && (i+1 < argc))
		{
			g_discover.m_name = malloc(strlen(argv[++i]) + 1);
			strcpy(g_discover.m_name, argv[i]); 
		}
	}
	if (g_discover.m_name == NULL)
	{
		g_discover.m_name = malloc(strlen(DEFAULTNAME) + 1);
		strcpy(g_discover.m_name, DEFAULTNAME); 
	}
	client_addconnector(g_client, connector);

	ret = client_main(g_client);

	client_destroy(g_client);
	servicesfactory_destroy(factory);
	free(g_discover.m_name);
	return ret;
}

/**
 * functions definition
 **/
int main_starteventlistener(char *service, int *eventPort)
{
	int ret = 0;
	strcpy(service, g_discover.m_name);
	*eventPort =g_discover.m_port;
	return ret;
}