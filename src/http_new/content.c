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
typedef struct _base ContentBase;

struct _base
{
	ContentBase *next;
	char *m_name;
	char *m_path;
	S_LibParser *m_lib;
	void *m_entity;
	int m_error;
};

struct _content
{
	int m_numbases;
	ContentBase *m_basefirst;
};

struct _content g_content =
{
	.m_numbases = 0,
	.m_basefirst = NULL,
};

S_LibParser *g_LibFirst = NULL;
static S_LibParser *contentfactory_createLib(char *p_LibId)
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
/**
 * factory
 **/

ContentFactory * contentfactory_new()
{
	ContentFactory *this = malloc(sizeof(ContentFactory));
	memset(this, 0, sizeof(ContentFactory));
	return this;
}

void contentfactory_destroy(ContentFactory *this)
{
	free(this);
	this = NULL;
}

int contentfactory_readconf(ContentFactory *this, char *filepath)
{
	FILE *fd = fopen(filepath, "r");

	if (fd != NULL)
	{
		char domainname[BUFFER_LEN], directory[BUFFER_LEN];
		while (fscanf(fd,"%0256s\t%0256s", domainname, directory) != EOF)
		{
			content_setbase(domainname, directory);
		}
		fclose(fd);
	}
	return 0;
}

void contentfactory_setbase(ContentFactory *this, char *p_name, char *p_path, char *p_libname)
{
	ContentBase *base;
	if (!g_content.m_basefirst)
	{
		this->m_basefirst = malloc(sizeof(ContentBase));
		base = this->m_basefirst;
	}
	else
	{
		base = this->m_basefirst;
		while(base->next) base =base->next;
		base->next = malloc(sizeof(ContentBase));
		base = base->next;
	}
	memset(base, 0, sizeof(ContentBase));
	base->m_name = p_name;
	base->m_path = p_path;
	log("create virtual server %s on directory %s\n", base->m_name, base->m_path);
	base->m_lib = contentfactory_createLib(base->m_libname);
	this->m_numbases ++;
}


ContentBase *contentfactory_get(ContentFactory *this, char *p_basename)
{
	
	ContentBase *base = this->m_basefirst;
	int ret = 404;
	S_LibParser *lib;
	char *basename=p_basename;
	p_basename=strchr(p_basename, ':');
	if (p_basename)
	{
		*p_basename++ = 0;
		
	}
	else
		p_basename=basename+strlen(p_basename);
	while (base)
	{
		if (strcmp(base->m_name, basename) == 0)
			break;
		base = base->next;
	}
	if (!base)
	{
		log("service (%s) not found\n", basename);
		return NULL;
	}
	return base;
}

/**
 * entity
 **/
struct _entity
{
	char *m_filepath;
	char *m_type;
	char *m_size;
};
typedef struct _entity Entity;

Entity *entity_new()
{
	Entity *this = malloc(sizeof(Entity));
	memset(this, 0, sizeof(Entity));
	return this;
}

void entity_destroy(Entity *this)
{
	free(this);
	this = NULL;
}

int entity_length(Entity *this)
{
	return this->m_size;
}

char *entity_type(Entity *this)
{
	return this->m_type;
}

/**
 * data access
 **/
Entity *contentbase_request(ContentBase *this, HttpRequest *request)
{
	Entity *entity = NULL;
	lib = this->m_lib;

	if ((entity =  lib->searchEntity(lib->data, utf8parsing(request->m_filepath))) != NULL)
	{
		this->m_error = lib->runEntity(entity, utf8parsing(request->m_get), scite	request->m_post);
		entity->m_type = lib->typeEntity(entity);
		entity->m_size = lib->sizeEntity(entity);
	}

	return entity;
}

int contentbase_data(ContentBase *this, Entity *entity, char *p_buffer, int p_len)
{
    S_LibParser *lib = this->m_lib;
    return lib->readEntity(entity, p_buffer, p_len);
}
