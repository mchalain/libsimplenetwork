/**
 * Project: libserver
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: clientadapter.c
 * description: this object manages the client socket on the server side.
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
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include "service_private.h"
#include <mpilote/clientadapter.h>
#include <mpilote/debug.h>

struct _ClientAdapter
{
	Service* m_service;
	int m_fd;
	struct sockaddr *m_peeraddr;
	socklen_t m_peeraddrlen;
	int m_connected;
};

/**
 * global variables
 **/
/**
 * Local functions
 **/
int clientadapter_setup(ClientAdapter *this);

/**
 * function declaration
 **/
/**
 * @brief class creator
 * @param service the service connected to the client socket
 * @param clientfd the client socket descriptor
 * @param fds the file descriptor list that trigg the event on the main socket
 **/
ClientAdapter *clientadapter_new(Service *service, int clientfd)
{
	ClientAdapter *this = malloc(sizeof(ClientAdapter));
	memset(this, 0, sizeof(ClientAdapter));
	this->m_fd = clientfd;
	this->m_service = service;

	this->m_peeraddrlen = sizeof(struct sockaddr_in);
	this->m_peeraddr = malloc(this->m_peeraddrlen);
	memset(this->m_peeraddr, 0, this->m_peeraddrlen);

	service_attach(service, this);
	return this;
}

int clientadapter_getsocket(ClientAdapter *this)
{
	return this->m_fd;
}

#define DEPRECATED
int clientadapter_getinfo(ClientAdapter *this, char *hostname, int len)
{
	char service[NI_MAXSERV];
	int ret = 0;
	struct sockaddr_storage peeraddr;
	int peeraddrlen = sizeof(struct sockaddr_storage);

	if ((ret = getsockname(this->m_fd, (struct sockaddr*)&peeraddr, &peeraddrlen)) < 0)
	{
		error("erro getsockname : %s\n", strerror(errno));
	}
	else if ((ret = getnameinfo((struct sockaddr*)&peeraddr, peeraddrlen, hostname, len, NULL, 0, 0)) < 0)
	{
		error("erro getnameinfo : %s\n", strerror(errno));
	}
/*
	else
	{
#ifndef DEPRECATED
		inet_ntop(peeraddr->, peeraddr,hostname, len);
#else
		strncpy(hostname, inet_ntoa(((struct sockaddr_in*)peeraddr)->sin_addr), len);
#endif
	}
*/
	return ret;
}

ClientAdapter *clientadapter_start(ClientAdapter *this)
{
	ClientAdapter *ret =NULL;
	log("%s\n", __FUNCTION__);
	if (clientadapter_setup(this) > 0)
		ret = this;
	this->m_connected = 1;
	log("connect\n");
	return ret;
}

Service *clientadapter_service(ClientAdapter *this)
{
	return this->m_service;
}

ClientAdapter *clientadapter_startthreaded(ClientAdapter *this)
{
	ClientAdapter *ret =NULL;
	int cpid;
	cpid = fork();
	if (cpid == 0)
	{
		if (clientadapter_setup(this) == 0)
		do
		{
			fd_set rfds;
			int ret;

			FD_ZERO(&rfds);
			FD_SET(this->m_fd,&rfds);

			ret = select(this->m_fd + 1, &rfds, NULL, NULL, NULL);

			if (ret > 0 && FD_ISSET(this->m_fd, &rfds))
			{
				if (service_main(this->m_service) < 0)
				{
					break;
				}
			}
		}
		while(1);
		clientadapter_destroy(this);
		_exit(0);
	}
	log("connect\n");
	return ret;
}

int clientadapter_setup(ClientAdapter *this)
{
	int opt = 0;

	log ("%s\n", __FUNCTION__);
	if (this->m_fd && service_getsocketopt(this->m_service, SERVICEOPT_F_SETFL, &opt))
		if ( fcntl(this->m_fd, F_SETFL, opt) < 0)
		{
			error("error non block\n");
			close(this->m_fd);
			this->m_fd = 0;
		}

	if (this->m_fd && service_getsocketopt(this->m_service, SERVICEOPT_SO_REUSEADDR, &opt))
		if ( setsockopt(this->m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			error("error reuse opt\n");
			close(this->m_fd);
			this->m_fd = 0;
		}

	if (this->m_fd && service_getsocketopt(this->m_service, SERVICEOPT_SO_KEEPALIVE, &opt))
		if (setsockopt(this->m_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
		{
			error("error keep alive opt\n");
			close(this->m_fd);
			this->m_fd = 0;
		}

	if (this->m_fd && service_getsocketopt(this->m_service, SERVICEOPT_SO_BROADCAST, &opt))
		if (setsockopt(this->m_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
		{
			error("error broadcast opt\n");
			close(this->m_fd);
			this->m_fd = 0;
		}

   /*
	if (this->m_fd && service_getsocketopt(this->m_service, SERVICEOPT_SO_BROADCAST, &opt))
		if((setsockopt(this->m_fd,SOL_SOCKET,SO_DONTROUTE, &opt,sizeof(opt))) == -1)
		{
			error("error don't route opt\n");
			close(this->m_fd);
			this->m_fd = 0;
		}
   }*/
	return this->m_fd;
}

int clientadapter_read(ClientAdapter *this, char *buffer, int bufferlen)
{
	int ret = 0;
	log("%s\n", __FUNCTION__);
	if (service_isstream(this->m_service))
	{
		ret = read(this->m_fd, buffer, bufferlen);
	}
	else
	{
		ret = recvfrom(this->m_fd, buffer, bufferlen, 0, this->m_peeraddr, &this->m_peeraddrlen);
	}
	if (ret < 0)
		ret = -errno;
	else if (ret == 0)
	{
		// end of socket
		ret = -1;
	}
	else if (ret < bufferlen)
		buffer[ret] = 0;
	return ret;
}

int clientadapter_write(ClientAdapter *this, char *buffer, int bufferlen)
{
	int ret = 0;

	if (this->m_connected == 1)
	{
		ret = write(this->m_fd, buffer, bufferlen);
	}
	else
	{
		ret = sendto(this->m_fd, buffer, bufferlen, 0, this->m_peeraddr, this->m_peeraddrlen);
	}
	if (ret < 0)
	{
		error("error write : %s\n", strerror(errno));
		ret = -errno;
	}
	else if (ret == 0)
	{
		log("%s\n", "socket closed");
		ret = -1;
	}
	return ret;
}

void clientadapter_destroy(ClientAdapter *this)
{
	shutdown(this->m_fd,SHUT_RDWR);
	close(this->m_fd);

	service_destroy(this->m_service);
	free(this);
	log("disconnect\n");
}

