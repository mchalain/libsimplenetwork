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
#include <mpilote/servicesfactory.h>
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>

#include "content.h"
#include "libparser.h"

#define SERVICENAME "http\0"
char *g_SERVICENAME = NULL;

#define BUFFER_LEN 256

#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
#define STD_STPCPY
#endif
struct _http
{
	char *m_filerequest;
	char *m_postrequest;
	char *m_hostrequest;
	char *m_useragent;
	E_Cmd m_cmd;
	struct
	{
		enum {E_Normal, E_Form} m_type;
		int m_len;
		char *m_data;
	} m_content;
};

int g_debug = 0;

typedef struct _http S_HttpSession;

typedef enum { E_ParseHeader, E_ParseBody, E_ParseEnd } E_ParseState;

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

Service *servicehttp_new(int serviceid);
Service_New *servicelib_new = servicehttp_new;

static int servicehttp_main(Service *this);
static void servicehttp_destroy(Service *this);
static char *servicehttp_getname(Service *this);

static struct _ServiceSingle
{
	char m_initialized;
	char m_name[5];
} g_serviceSingle = {0};

static void  __attribute__ ((constructor)) servicelib_initiate()
{
	log ("%s\n", __FUNCTION__);
	if (g_serviceSingle.m_initialized)
		return;

	strncpy(g_serviceSingle.m_name, SERVICENAME, sizeof(g_serviceSingle.m_name));

	g_serviceSingle.m_initialized = 1;

	if (!access("/etc/minihttp.conf", R_OK))
		content_readconf( "/etc/minihttp.conf");
}

Service *servicehttp_new(int serviceid)
{
	log ("%s\n", __FUNCTION__);
	servicelib_initiate();
	Service *this = service_new(e_service_tcp);

	this->m_port = 80;
	this->f_ops.main = servicehttp_main;
	this->f_ops.getname = servicehttp_getname;
	this->f_ops.destroy = servicehttp_destroy;

	return this;
}

static void servicehttp_destroy(Service *this)
{
	log("%s\n", __FUNCTION__);
}

static int servicehttp_main(Service *this)
{
	char input[BUFFER_LEN];
	char output[BUFFER_LEN];
	int ret;
	E_ParseState state = E_ParseHeader;
	S_HttpSession *http;

	memset(input, 0, sizeof(input));
	memset(output, 0, sizeof(output));

	if (this->m_client == NULL)
	{
		log("service not attached to the client\n");
		return -1;
	}
	http = httpsession_new();
	this->m_privatedata = (void *)http;
	while ((ret = clientadapter_read(this->m_client, input, sizeof(input))) > 0)
	{
		input[ret] = 0;
		state = httpsession_parseRequest(http, state, input, ret);
		if (ret < sizeof(input) || state == E_ParseEnd)
			break;
	}
	if (ret == -EAGAIN)
	{
		ret = 0;
		printf("try again\n");
		return ret;
	}
	while((ret = httpsession_buildResponse(http, output, sizeof(output) - 1)) > 0)
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
	log("%s\n", __FUNCTION__);
	return g_serviceSingle.m_name;
}

/**
 * HttpSession
 **/
static S_HttpSession *httpsession_new()
{
	S_HttpSession *http;
 	http = malloc(sizeof(S_HttpSession));
	memset(http, 0, sizeof(S_HttpSession));
	http->m_cmd = E_Unknown;
	http->m_filerequest=NULL;
	http->m_postrequest=NULL;
	http->m_hostrequest=NULL;
	http->m_content.m_type = E_Normal;
	http->m_content.m_len = 0;
	http->m_content.m_data = NULL;
	return http;
}

static void httpsession_destroy(S_HttpSession *p_http)
{
	if (p_http->m_filerequest)
	{
		free(p_http->m_filerequest);
		p_http->m_filerequest = NULL;
	}
	if (p_http->m_postrequest)
	{
		free(p_http->m_postrequest);
		p_http->m_postrequest = NULL;
	}
	if (p_http->m_hostrequest)
	{
		free(p_http->m_hostrequest);
		p_http->m_hostrequest = NULL;
	}
	if (p_http->m_useragent)
	{
		free(p_http->m_useragent);
		p_http->m_useragent = NULL;
	}
	if (p_http->m_content.m_data)
	{
		free(p_http->m_content.m_data);
		p_http->m_content.m_data = NULL;
		p_http->m_content.m_len = 0;
	}
}

static E_ParseState httpsession_parseRequest(S_HttpSession *p_http, E_ParseState p_state, char *p_buffer, int p_length)
{
	char *ptr = p_buffer;

	if (p_length == 0)
		return 0;
    if (p_state == E_ParseHeader)
    {
        while (*ptr != 0)
        {
            if (!strncmp(ptr, "GET ", 4))
            {
                p_http->m_cmd = E_Get;
                ptr += 4;
                ptr = strstore(&p_http->m_filerequest, ptr, " ?\n\r");
                if (*ptr == '?')
                {
                    ptr++;
                    ptr = strstore(&p_http->m_postrequest, ptr, " \n\r");
                }
            }
            else if (!strncmp(ptr, "HEAD ", 5))
            {
                p_http->m_cmd = E_Head;
                ptr += 4;
                ptr = strstore(&p_http->m_filerequest, ptr, " ?\n\r");
                if (*ptr == '?')
                {
                    ptr++;
                    ptr = strstore(&p_http->m_postrequest, ptr, " \n\r");
                }
            }
            else if (!strncmp(ptr, "POST ", 5))
            {
                p_http->m_cmd = E_Post;
                ptr += 5;
                ptr = strstore(&p_http->m_filerequest, ptr, " ?\n\r");
            }
            else if (!strncmp(ptr, "User-Agent: ", 12))
            {
                ptr += 12;
                ptr = strstore(&p_http->m_useragent, ptr, "\n\r");
            }
            else if (!strncmp(ptr, "Host: ", 6))
            {
                ptr += 6;
                ptr = strstore(&p_http->m_hostrequest, ptr, ":\n\r");
            }
            else if (!strncmp(ptr, "Content-Type: ", 14))
            {
                ptr += 14;
                if (!strncmp(ptr, "application/x-www-form-urlencoded", 32))
                {
                    ptr += 32;
                    p_http->m_content.m_type = E_Form;
                }
            }
            else if (!strncmp(ptr, "Content-Length: ", 16))
            {
                ptr += 16;
                p_http->m_content.m_len = atoi(ptr);
            }
            else if (*ptr == '\n' || *ptr == '\r')
            {
                p_state = E_ParseBody;
                ptr++;
                break;
            }
            //go to end of line
            while (*ptr != '\n' && *ptr != '\r' && *ptr != 0) ptr++;
            //go to next line
            while (*ptr == '\n' || *ptr == '\r') ptr++;
        }
    }
    if (p_state == E_ParseBody && p_http->m_content.m_len > 0 && *ptr != 0)
    {
        int len;
        int datalen = 0;
        while (*ptr == '\n' || *ptr == '\r')
            ptr++;

        if (p_http->m_content.m_data)
            datalen = strlen(p_http->m_content.m_data);

        len = strlen(ptr) + datalen;
        if (len > datalen)
        {
            p_http->m_content.m_data = realloc(p_http->m_content.m_data, len + 1);
            strcpy(p_http->m_content.m_data + datalen, ptr);
            p_http->m_content.m_data[len] = 0;
        }
        if (len == p_http->m_content.m_len)
            p_state = E_ParseEnd;
	}

	return p_state;
}

static int httpsession_buildResponse(S_HttpSession *p_http, char *p_buffer, int p_len)
{
	int ret = 0;
	static enum {E_Status, E_Header, E_Content, E_End, E_Stop} state = E_Status;
	int status;

	switch (state)
	{
		case E_Status:
			status = content_request(p_http->m_cmd, utf8parsing(p_http->m_hostrequest), utf8parsing(p_http->m_filerequest),utf8parsing(p_http->m_postrequest), utf8parsing(p_http->m_content.m_data));
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
		break;
		case E_Header:
			ret = httpsession_responseHeader(p_http, p_buffer, p_len);
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

int httpsession_responseHeader(S_HttpSession *p_http, char *p_buffer, int p_len)
{
	char *ptr = p_buffer;
	char size[10];

	ptr = stpcpy(ptr, "Server: "TARGET"\n\r");
	ptr = stpcpy(ptr, "Accept-Ranges: bytes\n\r");
	ptr = stpcpy(ptr, "Content-Type: ");
	ptr = stpcpy(ptr, content_type());
	ptr = stpcpy(ptr, "\n\rContent-Length: ");
	sprintf(size, "%u", content_length());
	ptr = stpcpy(ptr, size);
	ptr = stpcpy(ptr, "\n\n");

	return strlen(p_buffer);
}

/**
 * std functions not availables
 **/
char *strstore(char **p_output, char *p_input, char *p_tags)
{
	char *last;
	char *end = strpbrk(p_input, p_tags);
	if (!end)
	{
		end = p_input;
		while (*end++) ;
	}
	if (p_output)
	{
		if (!*p_output)
		{
			*p_output = malloc(end - p_input + 1);
		}
		last = stpncpy(*p_output, p_input, end - p_input) - 1;
		*(last+1) = 0;
	}
	return end;
}

#ifndef STD_STPCPY
char *stpcpy(char *p_output, char *p_input)
{
	char *first = p_output;
	while(*p_input) *p_output++ = *p_input++;
	*p_output = 0;
	return p_output;
}

char *stpncpy(char *p_output, char *p_input, int p_len)
{
	char *first = p_input;
	while(*p_input && (p_input - first < p_len)) *p_output++ = *p_input++;
	*p_output = 0;
	return p_output;
}
#endif
char *utf8parsing(char *p_buffer)
{
	char *ptr = p_buffer;
	char *end = p_buffer;

	if (!p_buffer)
		return p_buffer;

	end += strlen(p_buffer);
	while ((ptr = index(ptr,'%')) != NULL)
	{
		char value;
		char *next;
		value = strtol(ptr + 1, &next, 16);
		*ptr = value;
		ptr++;
		memmove(ptr, next, end - next);
		end -= next - ptr;
		*end = 0;
	}
	return p_buffer;
}

