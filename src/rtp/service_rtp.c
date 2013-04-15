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

#include <mpilote/service.h>
#include <mpilote/servicesfactory.h>
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>

#define BUFFER_LEN    256

int serviceanymote_main(Service *this);
char *serviceanymote_getname(Service *this);
void serviceanymote_destroy(Service *this);

typedef struct
{
	int m_device;
	char *m_devicename;
	char *m_buffer;
	char m_name[9];
} ServiceDeviceData;

void  __attribute__ ((constructor)) serviceanymote_initiate()
{
}

void *serviceanymote_new(int serviceid);
Service_New *servicelib_new = serviceanymote_new;

void *serviceanymote_new(int serviceid)
{
	ServiceDeviceData *data;
	Service *this;

	this = service_new(e_service_tcp);
	this->m_port = 9105;
	this->f_ops.main = serviceanymote_main;
	this->f_ops.getname = serviceanymote_getname;
	this->f_ops.destroy = serviceanymote_destroy;

	this->m_privatedata = malloc(sizeof(ServiceDeviceData));
	memset(this->m_privatedata, 0, sizeof(ServiceDeviceData));
	data = (ServiceDeviceData *)this->m_privatedata;
	strncpy(data->m_name, SERVICENAME, 8);
	return this;
}

int serviceanymote_main(Service *this)
{
	int ret = 0;
	char buffer[BUFFER_LEN];
	ServiceDeviceData *data = (ServiceDeviceData *)this->m_privatedata;

	if ((ret = clientadapter_read(this->m_client, buffer, BUFFER_LEN - 1)) > 0)
	{
		RTPPackeet *packet = rptpacket_new(this->m_rtpchannel);
		rtppacket_parse(buffer, ret);

		write(data->m_device, buffer, ret);
		log("anymote event sended\n");
	}
	return ret;
}

char *serviceanymote_getname(Service *this)
{
	ServiceDeviceData *data = (ServiceDeviceData *)this->m_privatedata;
	return data->m_name;
}

void serviceanymote_destroy(Service *this)
{
	ServiceDeviceData *data = (ServiceDeviceData *)this->m_privatedata;

#ifdef DEVICEADAPTER
	int i;
	for (i = 0; i < DEVICEADAPTER_MAX; i++)
	{
		if (data->m_deviceadapter[i] == NULL )
			break;
		deviceadapter_destroy(data->m_deviceadapter[i]);
	}
#elif defined(MKFIFO)
	close(data->m_device);
	unlink(DEVICEFILE);
#endif
	free(data->m_buffer);
	free(data->m_devicename);
	free(data);
	free(this);
}
