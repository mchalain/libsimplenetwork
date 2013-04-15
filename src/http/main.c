/**
 * Project: minihttp
 * Author: Marc Chalain
 * file: main.c
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
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <errno.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/server.h>
#include <mpilote/connector.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/debug.h>

#include "content.h"

extern Service *servicehttp_new(int serviceid);

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
	int ret, i;
	int serviceid = 0;

	g_server = server_new();
	atexit(stop);
	signal(SIGINT, terminate);

	ServicesFactory *factory = servicesfactory_new();
	serviceid = servicesfactory_add(factory, NULL, servicehttp_new);
	Connector *connector = connector_new(factory, serviceid);
	for (i = 0; i < argc; i++)
	{
		if ((strncmp(argv[i], "--port", 6) == 0) && (i+1 < argc))
		{
			int port = atoi(argv[++i]);
			connector_setport(connector, port);
		}
		if (!strcmp(argv[i], "--base") && ++i < argc)
		{
			char *dir = NULL;
			if ((dir = strchr(argv[i], '=')) != NULL )
			{
				*dir++ = 0;
				content_setbase(argv[i], dir);
			}
			else
				content_setbase("locahost", argv[i]);
		}
		if (!strcmp(argv[i], "--conf") && ++i < argc)
		{
			if (!access(argv[i], R_OK))
			{
				content_readconf( argv[i]);
			}
		}
		if (!strcmp(argv[i], "-h"))
		{
			fprintf(stderr, "%s [-base <hostname=filedirectory>]\n", argv[0]);
		}
	}
	log("create socket\n");
	if (server_addconnector(g_server, connector))
		return -1;
	log("server running\n");
	ret = server_main(g_server);

	server_destroy(g_server);
	return ret;
}
