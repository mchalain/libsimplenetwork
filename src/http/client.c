#include "../include/service.h"

static char header[] = "GET /manual/images/feather.jpg HTTP/1.1\n\
Host: localhost\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; fr; rv:1.8.1.1) Gecko/20061208 Firefox/2.0.0.1\n\
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\n\
Accept-Language: fr,fr-FR;q=0.9,en-US;q=0.8,en;q=0.6,fr-fr;q=0.5,en-us;q=0.4,zh;q=0.3,zh-CN;q=0.1\n\
Accept-Encoding: gzip,deflate\n\
Accept-Charset: ISO-8859-15,utf-8;q=0.7,*;q=0.7\n\
Keep-Alive: 300\n\
Connection: keep-alive\n\
\n\
q=0.7\n\
Keep-Alive: 300\n\
Connection: keep-aldisconnect\n\
\n";


int service_init(int *p_psocktype, int *p_psockproto, int *p_psockport)
{
	return 0;
}

int service_setup(char *p_option, char *p_value)
{
	return 0;
}

int service_main(int p_fd)
{
	char buffer[7200];
	int len;
//	read(p_fd, buffer, 500);
//	printf("receive: %s\n", buffer);
	write(p_fd, header, sizeof(header));
	len = read(p_fd, buffer, 7200);
	printf("receive: %d\n###\n%s\n###\n", len, buffer);

	return 0;
}
