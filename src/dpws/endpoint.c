#include "messages.h"

void endpoint_fillBody(EndPoint *this, DiscoveryBody *body, int long)
{
	vector_reset(this->m_types);
	if (this->m_refid != E_WsddEPRefId_All)
		discoverybody_setEndPoint(body, this->m_refid);
	if (long)
	{
		E_WsddEPType type;
		while ((type = endpoint_types(ep)) != -1)
		{
			discoverybody_appendType(body, type);
		}
		char *scop;
		while ((scop = endpoint_scopes(ep)) != NULL)
			discoverybody_appendScopes(body, scop);
		char *url;
		if ((url = endpoint_url(ep)) != NULL)
			discoverybody_setURL(body, url);
	}
}

EndPoint *endpoint_new(EndPointRefId refid)
{
	EndPoint *this = malloc(sizeof(EndPoint));
	memset(this, 0, sizeof(EndPoint));
	memcpy(this->m_refid, refid, sizeof(EndPointRefId));
	return this;
}

void enpoint_appendType(EndPoint *this, E_WsddEPType type)
{
	vector_append(this->m_types, type);
}

E_WsddEPType endpoint_types(EndPoint *this)
{
	E_WsddEPType type = vector_elem(this->m_types);
	vector_next(this->m_types);
	return type;
}

void endpoint_appendScopes(EndPoint *this, char *scop)
{
	vector_append(this->m_scopes, scop);
}

char *endpoint_scopes(EndPoint *this)
{
	char *scope = vector_elem(this->m_scopes);
	vector_next(this->m_scopes);
	return scope;
}

void endpoint_setURL(EndPoint *this, char *url)
{
	if (this->m_url)
		free(this->m_url);
	this->m_url = malloc(strlen(url) + 1);
	strcpy(this->m_url, url);
}

char *endpoint_url(EndPoint *this)
{
	return this->m_url;
}

void endpoint_destroy(EndPoint *this)
{
	free(this);
}
