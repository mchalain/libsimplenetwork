/**
 * Project: server
 * Author: Marc Chalain
 * file: main.c
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
#include <dirent.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/server.h>
#include <mpilote/connector.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/debug.h>
#ifdef ANYMOTEADAPTER
#include <anymotectrl/deviceadapterfactory.h>
#endif

extern Service *servicepairing_new(int serviceid);

char g_PLUGINDIR[] = SHAREDIR;

/**
 * functions declaration
 **/

static int main_loadplugin(char *dirpath);

/**
 * globals declaration
 **/
static Server *g_server;
static ServicesFactory *g_factory;

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
extern Service *servicehttp_new(int serviceid);
extern Service *serviceanymote_new(int serviceid);
int main(int argc, char ** argv)
{
	int ret;
	int servicepairingid;

	g_server = server_new();
	atexit(stop);
	signal(SIGINT, terminate);

	g_factory = servicesfactory_new();
	servicepairingid = servicesfactory_add(g_factory, NULL, servicepairing_new);
	main_loadplugin(g_PLUGINDIR);

	Connector *connector = connector_new(g_factory, servicepairingid);
	connector_readparameters(connector, argv, argc);

	server_addconnector(g_server, connector);
	ret = server_main(g_server);

	server_destroy(g_server);
	return ret;
}

int main_starteventlistener(char *service, int *eventPort)
{
	int ret = -1;
	int serviceid = servicesfactory_getserviceid(g_factory, service);
	log("service %s serviceid %d\n",service, serviceid);
	if (serviceid)
	{

		Connector *connector = connector_new(g_factory, serviceid);
		if (connector == NULL)
			return -1;
		connector_setport(connector, *eventPort);
		log("socket %d created on port %d\n", connector_getsocket(connector), *eventPort);
		ret = server_addconnector(g_server, connector);
		if (ret < 0)
		{
			log("socket not added %d\n", ret);
			connector_destroy(connector);
			//connector alredy exists we can continue
			ret = 0;
		}
		else
			log("socket %d added server\n", connector_getsocket(connector));
	}
	else
	{
		log("service not found\n");
	}
	return ret;
}

static int main_loadplugin(char *dirpath)
{
	DIR *dir = opendir(dirpath);
	struct dirent *entry = NULL;
	if (!access(dirpath, X_OK))
		do
		{
			entry = readdir(dir);
			if (entry && strstr(entry->d_name, ".so") != NULL)
			{
				char *path = NULL;

				path = malloc(strlen(dirpath)+strlen(entry->d_name) + 2); /* malloc check: free OK*/
				sprintf(path,"%s/%s", dirpath, entry->d_name);
				fprintf(stderr, "found library %s\n", entry->d_name);

				servicesfactory_add(g_factory, path, NULL);

				free(path);
			}
		} while(entry != NULL);
	else
	{
		fprintf(stderr, "directory not found %s\n", dirpath);
		return -1;
	}
	return 0;
}
