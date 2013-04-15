#ifndef __LIBPARSER_H__
#define __LIBPARSER_H__

struct _s_LibParser;
typedef struct _s_LibParser S_LibParser;
typedef void* LibData;
typedef void* LibEntity;
struct _s_LibParser
{
    S_LibParser *next;
    int (*checkId)(char *p_id);
    LibData (*setup)(char *p_name, char *p_id);
    LibEntity(*searchEntity)(LibData p_data, char *p_Entity);
    int (*runEntity)(LibEntity p_data, char *p_get, char *p_post);
    int (*sizeEntity)(LibEntity p_data);
    char *(*typeEntity)(LibEntity p_data);
    int (*readEntity)(LibEntity p_data, char *p_buffer, int p_size);
    void *data;
};
extern S_LibParser HTMLLibParser;
#endif