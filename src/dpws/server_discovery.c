#include "../include/service.h"

#define DPWS_DISCOVERY_PORT 3702
#define DPWS_DISCOVERY_TYPE SOCK_DGRAM

int service_init(int *p_psocktype, int *p_psockproto, int *p_psockport)
{
	*p_psocktype = DPWS_DISCOVERY_TYPE;
	*p_psockproto = 0;
	*p_psockport = DPWS_DISCOVERY_PORT;
	return 0;
}

int service_setup(char *p_option, char *p_value)
{
	return 0;
}

int service_main(int p_fd)
{
	enum {
		State_Hello,
		State_Request,
		State_Stop,
	} state;
	char buffer[50];

    do
	{
		switch (state)
		{
			case State_Hello:
			break;
			case State_Request:
			break;
			case State_Stop:
			break;
		}
	}
	while (state != State_Stop);
	return 0;
}
