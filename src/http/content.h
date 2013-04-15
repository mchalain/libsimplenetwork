#ifndef __CONTENT_H__
#define __CONTENT_H__

typedef enum {E_Unknown, E_Get, E_Head, E_Post} E_Cmd;

int content_readconf(char *filepath);
void content_setbase(char *p_name, char *p_path);
int content_request(E_Cmd cmd, char * p_basename, char *p_filename, char *p_get, char *p_post);
int content_length(void);
char *content_type();
int content_data(char *p_buffer, int p_len);

#endif
