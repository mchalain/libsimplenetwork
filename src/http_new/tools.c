/**
 * Project: minihttpd
 * Author: Marc Chalain (Copyright (C) 2005)
 * file: service_http.c
 * description: this object gives simple service to the server.
 *
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/
#include <stdio.h>
#include <stdlib.h>
/**
 * std functions not availables
 **/
char *strstore(char **p_output, char *p_input, char *p_tags)
{
	char *last;
	char *end = strpbrk(p_input, p_tags);
	if (!end)
	{
		end = p_input;
		while (*end++) ;
	}
	if (p_output)
	{
		if (!*p_output)
		{
			*p_output = malloc(end - p_input + 1);
		}
		last = stpncpy(*p_output, p_input, end - p_input) - 1;
		*(last+1) = 0;
	}
	return end;
}

#ifndef STD_STPCPY
char *stpcpy(char *p_output, char *p_input)
{
	char *first = p_output;
	while(*p_input) *p_output++ = *p_input++;
	*p_output = 0;
	return p_output;
}

char *stpncpy(char *p_output, char *p_input, int p_len)
{
	char *first = p_input;
	while(*p_input && (p_input - first < p_len)) *p_output++ = *p_input++;
	*p_output = 0;
	return p_output;
}
#endif
char *utf8parsing(char *p_buffer)
{
	char *ptr = p_buffer;
	char *end = p_buffer;

	if (!p_buffer)
		return p_buffer;

	end += strlen(p_buffer);
	while ((ptr = index(ptr,'%')) != NULL)
	{
		char value;
		char *next;
		value = strtol(ptr + 1, &next, 16);
		*ptr = value;
		ptr++;
		memmove(ptr, next, end - next);
		end -= next - ptr;
		*end = 0;
	}
	return p_buffer;
}

