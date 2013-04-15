#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <mpilote/debug.h>
#include "libparser.h"

static int HTMLcheckId(char *p_id);
static void *HTMLsetup(char *p_name, char *p_id);
static void *HTMLsearchEntity(void *p_data, char *p_Entity);
static int HTMLrunEntity(void *p_data, char *p_get, char *p_post);
static int HTMLsizeEntity(void *p_data);
static char *HTMLtypeEntity(void *p_data);
static int HTMLreadEntity(void *p_data, char *p_buffer, int p_len);

S_LibParser HTMLLibParser =
{
    .checkId = HTMLcheckId,
    .setup = HTMLsetup,
    .searchEntity = HTMLsearchEntity,
    .runEntity = HTMLrunEntity,
    .readEntity = HTMLreadEntity,
    .sizeEntity = HTMLsizeEntity,
    .typeEntity = HTMLtypeEntity,
};

struct _s_htmlEntity
{
	char *m_path;
	int m_fd;
	struct stat m_stat;
	char m_type[256];
	int m_readlen;
};
typedef struct _s_htmlEntity S_HtmlEntity;

struct _s_htmldata
{
    char *m_path;
};
typedef struct _s_htmldata S_HtmlData;

static int HTMLcheckId(char *p_id)
{
    return 1;
}

static void *HTMLsetup(char *p_name, char *p_id)
{
    S_HtmlData *data = (S_HtmlData *)malloc(sizeof(S_HtmlData));
    int length = strlen(p_id);
    if (p_id[length - 1] == '/')
    {
        data->m_path = (char *)malloc(length + 1);
        strcpy(data->m_path, p_id);
    }
    else
    {
        length++;
        data->m_path = (char *)malloc(length + 1);
        strcpy(data->m_path, p_id);
        data->m_path[length] = '/';
    }
    data->m_path[length + 1] = '\0';

    return (void *)data;
}

static void *HTMLsearchEntity(void *p_data, char *p_Entity)
{
	S_HtmlData *data = (S_HtmlData *)p_data;
	int length = strlen(p_Entity);
	char *filename;
	S_HtmlEntity *entity = (S_HtmlEntity *)malloc(sizeof(S_HtmlEntity));

	if (p_Entity[length - 1] == '/')
	{
		length += strlen("/index.html");
		filename = (char *)malloc(length + 1);
		sprintf(filename, "%s%s", p_Entity, "/index.html");
	}
	else
	{
		filename = (char *)malloc(length + 1);
		sprintf(filename, "%s", p_Entity);
	}
	entity->m_readlen = 0;

	length += strlen(data->m_path);
	entity->m_path = (char *)malloc(length + 1);
	sprintf(entity->m_path, "%s%s", data->m_path, filename);

	if (access(entity->m_path, R_OK))
	{
		error("file %s not readable %s\n", entity->m_path, strerror(errno));
		goto HTMLsearchEntity_Error;
	}
	else
	{
		log("HTML search Entity: %s\n", entity->m_path);
		entity->m_fd = open(entity->m_path, O_RDONLY);
#ifdef NOT_IN_A_DYNAMIC_LIBRARY
		lstat(entity->m_path, &entity->m_stat);
#else
		entity->m_stat.st_size = lseek(entity->m_fd, 0, SEEK_END);
		lseek(entity->m_fd, 0, SEEK_SET);
#endif
	}
	free(filename);
	return entity;

HTMLsearchEntity_Error:
	free(filename);
	free(entity);
	return NULL;
}

static int HTMLrunEntity(void *p_data, char *p_get, char *p_post)
{
    return 200;
}

static int HTMLsizeEntity(void *p_data)
{
    S_HtmlEntity *entity = (S_HtmlEntity *)p_data;
    return entity->m_stat.st_size;
}

static char *HTMLtypeEntity(void *p_data)
{
    S_HtmlEntity *entity = (S_HtmlEntity *)p_data;
    char *ptr = rindex(entity->m_path, '.');

    strcpy(entity->m_type, "text/ascii");
    if (ptr)
    {
        if (!strncasecmp(ptr,".html",5) || !strncasecmp(ptr,".htm",4))
            strcpy(entity->m_type, "text/html");
        else if (!strncasecmp(ptr,".css",4))
            strcpy(entity->m_type, "text/css");
        else if (!strncasecmp(ptr,".png",4))
            strcpy(entity->m_type, "image/png");
        else if (!strncasecmp(ptr,".jpg",4))
            strcpy(entity->m_type, "image/jpeg");
        else if (!strncasecmp(ptr,".js", 3))
            strcpy(entity->m_type, "application/x-javascript");
    }
    return entity->m_type;
}

static int HTMLreadEntity(void *p_data, char *p_buffer, int p_len)
{
    S_HtmlEntity *data = (S_HtmlEntity *)p_data;
     int ret = read(data->m_fd, p_buffer, p_len);
    data->m_readlen += ret;
    if (ret < p_len)
    {
	close(data->m_fd);
        log ("file length reading: %d\n", data->m_readlen);
    }
    return ret;
}
