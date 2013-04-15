#ifndef __SERVER_H__
#define __SERVER_H__

#define MAX_CLIENT 255

struct _Server;
typedef struct _Server Server;

#include <mpilote/connector.h>

Server *server_new();
int server_main( Server *this);
void server_destroy(Server *this);
int server_addconnector(Server *this, Connector *connector);
void server_closeconnector(Server *this, Connector *connector);
int server_addclient(Server *this, ClientAdapter *client);
void server_closeclient(Server *this, ClientAdapter *client);
void server_readparameters(Server *this, char ** p_argv, int p_argc);
void server_stop(Server *this);

#endif