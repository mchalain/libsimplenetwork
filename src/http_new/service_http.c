/**
 * Project: minihttpd
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: service_http.c
 * description: this object gives simple service to the server.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <mpilote/service.h>
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>

#include "content.h"
#include "request.h"
#include "libparser.h"

#define SERVICENAME "http"
char g_SERVICENAME[] = SERVICENAME;

#define BUFFER_LEN 256

#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
#define STD_STPCPY
#endif
typedef enum { E_ParseAction, E_ParseHeader, E_ParseBody, E_ParseEnd } E_ParseState;

int g_debug = 0;

/**
 * functions declarations
 **/

static S_HttpSession *httpsession_new();
static void httpsession_destroy(S_HttpSession *p_http);
static E_ParseState httpsession_parseRequest(S_HttpSession *p_http, E_ParseState p_state, char *p_buffer, int p_length);
static int httpsession_buildResponse(S_HttpSession *p_http, char *p_buffer, int p_len);
static int httpsession_responseHeader(S_HttpSession *p_http, char *p_buffer, int p_len);

char *strstore(char **p_output, char *p_input, char *p_tags);
#ifndef STD_STPCPY
char *stpcpy(char *p_output, char *p_input);
char *stpncpy(char *p_output, char *p_input, int p_len);
#endif
char *utf8parsing(char *p_buffer);

static int servicehttp_main(Service *this);
static char *servicehttp_getname(Service *this);

/**
 * ServiceHttp
 **/
typedef struct _servicehttp
{
	ContentFactory *m_factory;
} ServiceHttp;

void *servicehttp_new(int serviceid)
{
	Service *this = service_new(e_service_tcp);
	ServiceHttp *data;

	this->m_port = 80;
	this->f_ops.main = servicehttp_main;
	this->f_ops.getname = servicehttp_getname;
	this->f_ops.destroy = servicehttp_destroy;
	data = this->m_privatedata = malloc(sizeof(ServiceHttp));
	memset(data , 0, sizeof(ServiceHttp));
	data->m_factory = contentfactory_new();
	
	return this;
}

static void servicehttp_destroy(Service *this)
{
	ServiceHttp *data = (ServiceHttp *)this->m_privatedata;
	contentfactory_destroy(data->m_factory);
	free(data);
	this->m_privatedata = NULL;
}

static int servicehttp_main(Service *this)
{
	ServiceHttp *data = (ServiceHttp *)this->m_privatedata;
	char input[BUFFER_LEN];
	char output[BUFFER_LEN];
	int ret;
	E_ParseState state = E_ParseAction;
	HttpRequest *request;

	memset(input, 0, sizeof(input));
	memset(output, 0, sizeof(output));

	if (this->m_client == NULL)
	{
		log("service not attached to the client\n");
		return -1;
	}

	request = httprequest_new(this->m_client);
	
	do
	{
		ret = clientadapter_read(this->m_client, input, sizeof(input));
		if (ret > 0)
		{
			int len = ret;
			input[len] = 0;
			ret = 0;
			while ((ret = httprequest_parse(request, input + ret, len - ret)) > 0);
		}
	} while (ret >= 0);
	
	if (ret == -EAGAIN)
	{
		ret = 0;
		printf("try again\n");
		return ret;
	}

	this->m_base = contentfactory_get(data->m_factory, request->m_host);
	entity = contentbase_request(p_http->m_base, request);
	while((ret = servicehttp_buildResponse(this, entity, output, sizeof(output) - 1)) > 0)
	{
		output[ret] = 0;
		clientadapter_write(this->m_client, output, ret);
	}
	httpsession_destroy(http);
	ret = -1;
	return ret;
}

static char *servicehttp_getname(Service *this)
{
	return g_SERVICENAME;
}

static int servicehttp_buildResponse(Service *this, Entity *p_entity, char *p_buffer, int p_len)
{
	int ret = 0;
	static enum {E_Status, E_Header, E_Content, E_End, E_Stop} state = E_Status;
	int status;

	switch (state)
	{
		case E_Status:
			if (p_entity)
			{
				ret = servicehttp_responseStatus(this, p_entity->m_status, p_buffer, p_len);
			}
			else
				ret = servicehttp_responseStatus(this, 501, p_buffer, p_len);
		break;
		case E_Header:
			ret = servicehttp_responseHeader(this, p_entity, p_buffer, p_len);
			if (p_http->m_cmd == E_Get || p_http->m_cmd == E_Post)
			{
				state = E_Content;
			}
			else
				state = E_End;
		break;
		case E_Content:
			ret = content_data(p_buffer, p_len);
			if (ret < p_len)
				state = E_Stop;
		break;
		case E_End:
			strcpy(p_buffer, "\n\r\n\r");
			ret = 2;
			state = E_Stop;
		break;
		case E_Stop:
			state = E_Status;
		break;
	}
	return ret;
}

int servicehttp_responseStatus(Service *this, int status, char *p_buffer, int len)
{
	int ret = 0;
	switch (status)
	{
		case 200:
		{
			strcpy(p_buffer, "HTTP/1.0 200 OK\n\r");
			state = E_Header;
		}
		break;
		case 404:
		{
			char *ptr = p_buffer;
			ptr = stpcpy(ptr, "HTTP/1.0 404 File Not Found\n\r");
			ptr = stpcpy(ptr, "Content-Length: 0\n\r");
			state = E_End;
		}
		break;
		case 501:
		{
			char *ptr = p_buffer;
			ptr = stpcpy(ptr, "HTTP/1.1 501 Not Implemented\n\r");
			state = E_End;
		}
	}
	ret = strlen(p_buffer);
	return ret;
}

int servicehttp_responseHeader(Service *this, Entity *p_entity, char *p_buffer, int p_len)
{
	char *ptr = p_buffer;
	char size[10];

	ptr = stpcpy(ptr, "Server: "TARGET"\n\r");
	ptr = stpcpy(ptr, "Accept-Ranges: bytes\n\r");
	ptr = stpcpy(ptr, "Content-Type: ");
	ptr = stpcpy(ptr, p_entity->m_type);
	ptr = stpcpy(ptr, "\n\rContent-Length: ");
	sprintf(size, "%u", p_entity->m_size);
	ptr = stpcpy(ptr, size);
	ptr = stpcpy(ptr, "\n\n");

	return strlen(p_buffer);
}

