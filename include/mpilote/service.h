#ifndef __SERVICE_H__
#define __SERVICE_H__


struct _Service;
typedef struct _Service Service;
#include <mpilote/clientadapter.h>

struct _Service
{
	ClientAdapter *m_client;
	int m_port;
	int m_type;
	int m_protocol;
	struct
	{
		int f_setfl;
		unsigned int so_reuseaddr:1;
		unsigned int so_keepalive:1;
		unsigned int so_broadcast:1;
	} m_opts;
	struct
	{
		int (*main)(Service *this);
		int (*request)(Service *this);
		char *(*getname)(Service *this);
		void(*destroy)(Service *this);
	} f_ops;
	void *m_privatedata;
};

typedef enum
{
	e_service_tcp,
	e_service_udp,
	e_service_broadcast,
	e_service_multicast,
} E_ServiceType;

Service *service_new(E_ServiceType servicetype);
#endif
