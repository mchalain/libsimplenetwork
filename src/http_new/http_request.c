/**
 * HttpSession
 **/
struct _http
{
	E_Cmd m_cmd;
	char *m_host;
	int m_port;
	char *m_filepath;
	char *m_useragent;
	struct
	{
		enum {E_File, E_Form} m_type;
		int m_len;
		char *m_data;
	} m_post;
};

typedef struct _http HttpRequest;

static HttpRequest *httprequest_new(ClientAdapter *client)
{
	HttpRequest *http;
 	http = malloc(sizeof(S_HttpSession));
	memset(http, 0, sizeof(S_HttpSession));
	return http;
}

static void httprequest_destroy(HttpRequest *p_http)
{
	if (p_http->m_filepath)
	{
		free(p_http->m_filepath);
		p_http->m_filepath = NULL;
	}
	if (p_http->m_get)
	{
		free(p_http->m_get);
		p_http->m_get = NULL;
	}
	if (p_http->m_host)
	{
		free(p_http->m_host);
		p_http->m_host = NULL;
	}
	if (p_http->m_useragent)
	{
		free(p_http->m_useragent);
		p_http->m_useragent = NULL;
	}
	if (p_http->m_post.m_data)
	{
		free(p_http->m_post.m_data);
		p_http->m_post.m_data = NULL;
		p_http->m_post.m_len = 0;
	}
	free(p_http);
	p_http = NULL;
}

static E_ParseState httprequest_parse(HttpRequest *p_http, E_ParseState p_state, char *p_buffer, int p_length)
{
	char *ptr = p_buffer;

	if (p_length == 0)
		return 0;
	switch (p_http->m_state)
	{
		case E_ParseAction
		{
			while (*ptr != 0)
			{
			if (!strncmp(ptr, "GET ", 4))
			{
				p_http->m_cmd = E_Get;
				ptr += 4;
			}
			else if (!strncmp(ptr, "HEAD ", 5))
			{
				p_http->m_cmd = E_Head;
				ptr += 4;
			}
			else if (!strncmp(ptr, "POST ", 5))
			{
				p_http->m_cmd = E_Post;
				ptr += 5;
			}
			ptr = strstore(&p_http->m_filepath, ptr, " ?\n\r");
			if (*ptr == '?')
			{
			    ptr++;
			    ptr = strstore(&p_http->m_get, ptr, " \n\r");
			}
			p_http->m_state = E_ParseHeader;
		}
		break;
		case E_ParseHeader:
		{
			if (!strncmp(ptr, "User-Agent: ", 12))
			{
				ptr += 12;
				ptr = strstore(&p_http->m_useragent, ptr, "\n\r");
			}
			else if (!strncmp(ptr, "Host: ", 6))
			{
				ptr += 6;
				ptr = strstore(&p_http->m_host, ptr, ":\n\r");
				if (*ptr == ':')
				{
				    ptr++;
				    ptr = strstore(&p_http->m_port, ptr, "\n\r");
				}
			}
			else if (!strncmp(ptr, "Content-Type: ", 14))
			{
				ptr += 14;
				if (!strncmp(ptr, "application/x-www-form-urlencoded", 32))
				{
					ptr += 32;
					p_http->m_post.m_type = E_Form;
				}
			}
			else if (!strncmp(ptr, "Content-Length: ", 16))
			{
				ptr += 16;
				p_http->m_post.m_len = atoi(ptr);
			}
			else if (*ptr == '\n' || *ptr == '\r')
			{
				p_http->m_state = E_ParseBody;
				ptr++;
			}
		}
		break;
		case E_ParseBody:
		{
			int len;
			int datalen = 0;
			while (*ptr == '\n' || *ptr == '\r')
			ptr++;

			if (p_http->m_post.m_data)
				datalen = strlen(p_http->m_post.m_data);

			len = strlen(ptr) + datalen;
			if (len > datalen)
			{
				p_http->m_post.m_data = realloc(p_http->m_post.m_data, len + 1);
				strcpy(p_http->m_post.m_data + datalen, ptr);
				p_http->m_post.m_data[len] = 0;
			}
			if (len == p_http->m_post.m_len)
				p_state = E_ParseEnd;
			}
		}
	}
	//go to end of line
	while (*ptr != '\n' && *ptr != '\r' && *ptr != 0) ptr++;
	//go to next line
	while (*ptr == '\n' || *ptr == '\r') ptr++;

	return p_state;
}

