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

#ifdef DEVICEADAPTER
# include <anymote/anymote.h>
# include <anymote/deviceadapter.h>
# include <anymote/deviceadapterfactory.h>
#endif

#define SERVICENAME "_anymote"
#ifndef DEVICEADAPTER
#define DEVICEFILE "/tmp/anymote"
#endif
#define BUFFER_LEN    256
#define DEVICENAME_MAXLEN 10
#define DEVICEADAPTER_MAX 5

#ifndef DEVICEADAPTER
int main_getpipe()
{
	int fd = 1;
#if defined(MKFIFO)
	mkfifo(DEVICEFILE, 0666);
	fd = open(DEVICEFILE, O_RDWR);
#endif
	return fd;
}

int main_setdevicename(char *buffer, int len)
{
	strncpy(buffer, "MPilote", len);
	return 0;
}
#endif

int serviceanymote_main(Service *this);
char *serviceanymote_getname(Service *this);
void serviceanymote_destroy(Service *this);

typedef struct
{
#ifdef DEVICEADAPTER
	DeviceAdapter *m_deviceadapter[DEVICEADAPTER_MAX];
#endif
	int m_device;
	char *m_devicename;
	char *m_buffer;
	char m_name[9];
} ServiceDeviceData;

#ifdef DEVICEADAPTER
void  __attribute__ ((constructor)) serviceanymote_initiate()
{
	printf("%s\n", __FUNCTION__);
	DeviceAdapterFactory_loadlibrary(NULL);
}
#endif

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
#ifdef DEVICEADAPTER
	{
		int i;
		for (i = 0; i < DEVICEADAPTER_MAX; i++)
			if ((data->m_deviceadapter[i] = DeviceAdapterFactory_create(i, "/dev/uinput")) == NULL) break;
	}
	data->m_devicename = malloc(DEVICENAME_MAXLEN);
	memset(data->m_devicename, 0, DEVICENAME_MAXLEN);
	deviceadapter_getname(data->m_deviceadapter[0], data->m_devicename, DEVICENAME_MAXLEN);
#else
	data->m_device = main_getpipe();
	data->m_devicename = malloc(DEVICENAME_MAXLEN);
	main_setdevicename(data->m_devicename, DEVICENAME_MAXLEN);
#endif
	return this;
}

int serviceanymote_main(Service *this)
{
	int ret = 0;
	char buffer[BUFFER_LEN];
	ServiceDeviceData *data = (ServiceDeviceData *)this->m_privatedata;

	if ((ret = clientadapter_read(this->m_client, buffer, BUFFER_LEN - 1)) > 0)
	{
#ifdef DEVICEADAPTER
		int i;
		AnymoteMessage *message = anymotemessage_new(REMOTEMESSAGE);
		log("new anymote message\n");
		anymotemessage_read(message, data->m_buffer, ret);
		for (i = 0; i < DEVICEADAPTER_MAX; i++)
			if (data->m_deviceadapter[i] == NULL || deviceadapter_parse(data->m_deviceadapter[i], message) >= 0 ) break;
		//anymotemessage_print(message);
#else
		write(data->m_device, buffer, ret);
		log("anymote event sended\n");
#endif
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
