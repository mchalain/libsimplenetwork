int service_setup(char *p_option, char *p_value)
{
	return 0;
}

int service_main(int p_fd)
{
	int ret = 0, value;

	if ((value = waitheader(p_fd)) < 0)
		ret = value;
	else if ((value = asknbmessages(p_fd) < 0)
		ret = value;
	return ret;
}
