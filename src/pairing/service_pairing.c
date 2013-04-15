#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>

#define BUFFER_LEN    256
#define MAX_PING_TOKS 5
#define SERVICENAME "pairing"

static struct _ServiceSingle
{
	char m_initialized;
	char *m_hostname;
	char m_name[8];
} g_serviceSingle = {0};

static void  __attribute__ ((constructor)) servicelib_initiate()
{
	log ("%s\n", __FUNCTION__);
	if (g_serviceSingle.m_initialized)
		return;

	strncpy(g_serviceSingle.m_name, SERVICENAME, sizeof(g_serviceSingle.m_name));

	g_serviceSingle.m_initialized = 1;

#ifndef HOSTNAME
	g_serviceSingle.m_hostname = getenv("PAIRINGNAME");
#endif
	if (g_serviceSingle.m_hostname == NULL)
	{
		g_serviceSingle.m_hostname = malloc(BUFFER_LEN);
#ifndef HOSTNAME
		gethostname(g_serviceSingle.m_hostname, BUFFER_LEN - 1);
#else
		strncpy(g_serviceSingle.m_hostname, HOSTNAME, BUFFER_LEN - 1);
#endif
	}
}

typedef struct
{
	char *m_discovername;
	int m_discoverport;
	char m_name[8];
} ServicePairingData;

extern int main_starteventlistener(char *service, int *eventPort);

int servicepairing_main(Service *this);
int servicepairing_request(Service *this);
char *servicepairing_getname(Service *this);

void *servicepairing_new(int serviceid)
{
	Service *this;

	log("%s\n", __FUNCTION__);
	servicelib_initiate();
	this = service_new(e_service_broadcast);
	this->m_port = 9101;
	this->f_ops.main = servicepairing_main;
	this->f_ops.request = servicepairing_request;
	this->f_ops.getname = servicepairing_getname;
	ServicePairingData *data = malloc(sizeof(ServicePairingData));
	memset(data, 0, sizeof(ServicePairingData));
	this->m_privatedata = data;
	strcpy(data->m_name, "pairing");
	return this;
}

char *servicepairing_getname(Service *this)
{
	ServicePairingData *data = this->m_privatedata;
	return data->m_name;
}

int servicepairing_main(Service *this)
{
	int ret = 0;
	char       buffer[BUFFER_LEN];
	char                reply[BUFFER_LEN];
	char*               pingTokens[MAX_PING_TOKS];
	int			nbTokens =  0;

	ret = clientadapter_read(this->m_client, buffer, BUFFER_LEN - 1);
	if (ret > 0)
	{
		log ("%s\n", __FUNCTION__);
		// Tokenise buffer
		int eventPort;
		char *service = NULL;
		long tok;
		char* pch = strtok((char*)buffer, " ");
		for( tok=0; pch != NULL && tok < MAX_PING_TOKS; tok++)
		{
			pingTokens[tok] = pch;
			nbTokens++;
			pch = strtok(NULL, " ");
		}
		if (!strncmp(pingTokens[0], "discover", 9))
		{
			eventPort = atoi(pingTokens[2]);
			service = malloc(strlen(pingTokens[1]) + 1);
			strcpy(service, pingTokens[1]);
			ret = main_starteventlistener(service, &eventPort);
		}
		memset(reply, 0, BUFFER_LEN);
		if (ret == 0)
		{
			snprintf(reply, BUFFER_LEN - 1, "%s %s %d", service, g_serviceSingle.m_hostname, eventPort);
			log("reply: %s\n", reply);
			clientadapter_write(this->m_client, reply, strlen(reply) + 1);
		}
		else
		{
			snprintf(reply, BUFFER_LEN - 1, "%s %s %s", service, g_serviceSingle.m_hostname, "unknown");
			log("reply: %s\n", reply);
			clientadapter_write(this->m_client, reply, strlen(reply) + 1);
		}
		if (service)
			free(service);
		// the server want to know if there is a new file descriptor to check
		ret = (ret == 0)? 1 : 0;
	}
	return ret;
}

int servicepairing_request(Service *this)
{
	int ret = 0;
	char       buffer[BUFFER_LEN];
	static int state = 0;
	ServicePairingData *data = this->m_privatedata;

	log("%s\n", __FUNCTION__);
	if (state == 0)
	{
		data->m_discovername = malloc(BUFFER_LEN + 1);
		memset(data->m_discovername, 0, BUFFER_LEN);
		if (main_starteventlistener(data->m_discovername, &data->m_discoverport) < 0)
			ret = -1;
		else
		{
			snprintf(buffer, BUFFER_LEN - 1, "%s %s %d", "discover", data->m_discovername, data->m_discoverport);
			ret = strlen(buffer);
			log("send : %s \n", buffer);
			if ((ret = clientadapter_write(this->m_client, buffer, ret))> 0 )
			{
				log("sent\n");
				state = 1;
			}
			else
			{
				error("error write socket : %s\n", strerror(errno));
				ret = -1;
			}
		}
	}
	else if (state == 1)
	{
		do 
		{
			if ((ret = clientadapter_read(this->m_client, buffer, sizeof(buffer))) > 0)
			{
				char hostname[BUFFER_LEN];
				clientadapter_getinfo(this->m_client, hostname, BUFFER_LEN);
				char*               pingTokens[MAX_PING_TOKS];
				int			nbTokens =  0;
				long tok;
				char* pch = strtok((char*)buffer, " ");
				for( tok=0; pch != NULL && tok < MAX_PING_TOKS; tok++)
				{
					pingTokens[tok] = pch;
					nbTokens++;
					pch = strtok(NULL, " ");
				}
				if (!strncmp(pingTokens[2], "unknown", 7))
					printf("service %s not found on %s\n", pingTokens[0], hostname);
				else
					printf("%s server %s %s:%d\n", pingTokens[0], pingTokens[1], hostname, atoi(pingTokens[2]));
			}
			else if ( ret != -EAGAIN)
			{
				error("error read socket : %s\n", strerror(errno));
			}
		} while (ret > 0);
		ret = -1; //because the dialogue is complete
		state = 2;
	}
	return ret;
}
