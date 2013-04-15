#ifndef __CLIENT_H__
#define __CLIENT_H__

#define MAX_CLIENT 255

struct _Client;
typedef struct _Client Client;

#include <mpilote/servicesfactory.h>

Client *client_new();
int client_main( Client *this);
void client_destroy(Client *this);
int client_addconnector(Client *this, Connector *connector);
void client_closeconnector(Client *this, Connector *connector);
void client_stop(Client *this);
#endif