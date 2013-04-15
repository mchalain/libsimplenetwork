#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <mpilote/debug.h>
#include "content.h"
#include "libparser.h"

#define BUFFER_LEN 256

struct _base;
typedef struct _base S_Base;

struct _base
{
    S_Base *next;
    char *m_name;
    char *m_path;
    S_LibParser *m_lib;
};

struct _content
{
	int m_numbases;
	S_Base *m_basefirst;
    S_Base *m_curBase;
    void *m_entity;
};

struct _content g_content =
{
	.m_numbases = 0,
    .m_basefirst = NULL,
    .m_curBase = NULL,
};

S_LibParser *g_LibFirst = NULL;
S_LibParser *content_createLib(char *p_LibId, char *p_name)
{
    S_LibParser *lib = g_LibFirst;
    while(lib)
    {
	if (lib->checkId(p_LibId))
	{
	    break;
	}
	lib = lib->next;
    }
    if (!lib)
	lib = &HTMLLibParser;
    lib->data = lib->setup(p_name, p_LibId);
    return lib;
}

int content_readconf(char *filepath)
{
	FILE *fd = fopen(filepath, "r");

	if (fd != NULL)
	{
		char domainname[BUFFER_LEN], directory[BUFFER_LEN];
		while (fscanf(fd,"%0256s\t%0256s", domainname, directory) != EOF)
		{
			if (domainname[0] != '#')
			{
				log("new line\n");
				content_setbase(domainname, directory);
			}
		}
		fclose(fd);
	}
	return 0;
}

void content_setbase(char *p_name, char *p_path)
{
	S_Base *base;
	if (g_content.m_basefirst == NULL)
	{
		g_content.m_basefirst = malloc(sizeof(S_Base));
		base = g_content.m_basefirst;
	}
	else
	{
		base = g_content.m_basefirst;
		while(base->next) base =base->next;
		base->next = malloc(sizeof(S_Base));
		base = base->next;
	}
	memset(base, 0, sizeof(S_Base));
	base->m_name = malloc(strlen(p_name)+1);
	strcpy(base->m_name, p_name);
	base->m_path = malloc(strlen(p_path)+1);
	strcpy(base->m_path, p_path);
	log("create virtual server %s on directory %s\n", base->m_name, base->m_path);
	base->m_lib = content_createLib(base->m_path, base->m_name);
	g_content.m_numbases ++;
}

int content_request(E_Cmd cmd, char * p_basename, char *p_filename, char *p_get, char *p_post)
{
	S_Base *base = g_content.m_basefirst;
	int ret = 404;
	S_LibParser *lib;
	char *basename=p_basename;

	if (base == NULL)
		content_setbase("localhost", "/home/http");
	if (cmd == E_Unknown)
		return 501;
	while (base)
	{
		int len;
		if (base->m_name[0] == '*' )
			base->m_name++;
		len = strlen(base->m_name);
		if (strncmp(base->m_name, basename, len) == 0)
			break;
		base = base->next;
	}
	if (!base)
    {
	log("service (%s) not found\n", basename);
	return 404;
    }

    g_content.m_curBase = base;
    lib = g_content.m_curBase->m_lib;

    if ((g_content.m_entity =  lib->searchEntity(lib->data, p_filename)) != NULL)
    {
	ret = lib->runEntity(g_content.m_entity, p_get, p_post);
    }

    return ret;
}

int content_length(void)
{
    S_LibParser *lib = g_content.m_curBase->m_lib;
    int size = lib->sizeEntity(g_content.m_entity);
	return size;
}

char *content_type()
{
    S_LibParser *lib = g_content.m_curBase->m_lib;
    char *type = lib->typeEntity(g_content.m_entity);
	return type;
}

int content_data(char *p_buffer, int p_len)
{
    S_LibParser *lib = g_content.m_curBase->m_lib;
    return lib->readEntity(g_content.m_entity, p_buffer, p_len);
}
