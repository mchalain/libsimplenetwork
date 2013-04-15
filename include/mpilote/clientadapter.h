#ifndef __CLIENTADAPTER_H__
#define __CLIENTADAPTER_H__

struct _ClientAdapter;
typedef struct _ClientAdapter ClientAdapter;
#include <mpilote/service.h>

ClientAdapter *clientadapter_new(Service *service, int clientfd);
int clientadapter_getsocket(ClientAdapter *this);
int clientadapter_getinfo(ClientAdapter *this, char *hostname, int len);
ClientAdapter *clientadapter_start(ClientAdapter *this);
Service *clientadapter_service(ClientAdapter *this);
int clientadapter_main(ClientAdapter *this);
int clientadapter_read(ClientAdapter *this, char *buffer, int bufferlen);
int clientadapter_write(ClientAdapter *this, char *buffer, int bufferlen);
void clientadapter_destroy(ClientAdapter *this);

#endif